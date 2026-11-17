#include <furi.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "C3Network"
#define UART_BAUD 115200U
#define RX_BUFFER_SIZE 512U
#define RX_BUFFER_MASK (RX_BUFFER_SIZE - 1U)
#define RX_LINE_SIZE 160U
#define MAX_NETWORKS 20U
#define VISIBLE_NETWORKS 4U
#define SSID_SIZE 33U
#define BSSID_SIZE 18U

typedef struct {
    char ssid[SSID_SIZE];
    char bssid[BSSID_SIZE];
    int16_t rssi;
    uint8_t channel;
    uint8_t auth;
} C3NetworkAp;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMutex* mutex;
    FuriHalSerialHandle* serial;

    volatile bool running;
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
    uint8_t rx_buffer[RX_BUFFER_SIZE];

    char rx_line[RX_LINE_SIZE];
    size_t rx_line_length;

    char pending_command[8];
    bool command_pending;

    bool show_results;
    bool show_details;
    bool scanning;
    uint8_t menu_index;

    uint16_t network_total;
    uint8_t network_count;
    uint8_t selected_network;
    uint8_t scroll_offset;
    C3NetworkAp networks[MAX_NETWORKS];

    char last_message[32];
} C3NetworkApp;

static const char* c3_network_auth_name(uint8_t auth) {
    switch(auth) {
    case 0: return "OPEN";
    case 1: return "WEP";
    case 2: return "WPA-PSK";
    case 3: return "WPA2-PSK";
    case 4: return "WPA/WPA2";
    case 5: return "WPA2-ENT";
    case 6: return "WPA3-PSK";
    case 7: return "WPA2/WPA3";
    case 8: return "WAPI";
    default: return "UNKNOWN";
    }
}

static void c3_network_uart_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    UNUSED(event);
    C3NetworkApp* app = context;

    while(furi_hal_serial_async_rx_available(handle)) {
        const uint8_t byte = furi_hal_serial_async_rx(handle);
        const uint16_t next = (uint16_t)((app->rx_head + 1U) & RX_BUFFER_MASK);

        if(next != app->rx_tail) {
            app->rx_buffer[app->rx_head] = byte;
            app->rx_head = next;
        }
    }
}

static void c3_network_parse_ap(C3NetworkApp* app, const char* line) {
    if(app->network_count >= MAX_NETWORKS) return;

    const char* ssid_start = strstr(line, "ssid=\"");
    const char* bssid_start = strstr(line, "bssid=");
    const char* rssi_start = strstr(line, "rssi=");
    const char* channel_start = strstr(line, "channel=");
    const char* auth_start = strstr(line, "auth=");

    if(!ssid_start || !bssid_start || !rssi_start || !channel_start || !auth_start) {
        return;
    }

    ssid_start += 6;
    const char* ssid_end = strchr(ssid_start, '"');
    if(!ssid_end) return;

    C3NetworkAp* ap = &app->networks[app->network_count];
    const size_t ssid_length = (size_t)(ssid_end - ssid_start);
    if(ssid_length == 0) {
        snprintf(ap->ssid, sizeof(ap->ssid), "<hidden>");
    } else {
        const size_t copy_length = ssid_length < (SSID_SIZE - 1U) ?
                                       ssid_length :
                                       (SSID_SIZE - 1U);
        memcpy(ap->ssid, ssid_start, copy_length);
        ap->ssid[copy_length] = '\0';
    }

    bssid_start += 6;
    const char* bssid_end = strchr(bssid_start, ' ');
    if(!bssid_end) bssid_end = bssid_start + strlen(bssid_start);
    size_t bssid_length = (size_t)(bssid_end - bssid_start);
    if(bssid_length >= sizeof(ap->bssid)) bssid_length = sizeof(ap->bssid) - 1U;
    memcpy(ap->bssid, bssid_start, bssid_length);
    ap->bssid[bssid_length] = '\0';

    ap->rssi = (int16_t)atoi(rssi_start + 5);
    ap->channel = (uint8_t)atoi(channel_start + 8);
    ap->auth = (uint8_t)atoi(auth_start + 5);
    app->network_count++;
}

static void c3_network_process_line(C3NetworkApp* app, const char* line) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    if(strcmp(line, "PONG") == 0) {
        snprintf(app->last_message, sizeof(app->last_message), "PONG received");
    } else if(strncmp(line, "STATUS ", 7) == 0) {
        snprintf(app->last_message, sizeof(app->last_message), "ESP32-C3 online");
    } else if(strcmp(line, "SCAN_BEGIN") == 0) {
        app->scanning = true;
        app->show_results = true;
        app->show_details = false;
        app->network_total = 0;
        app->network_count = 0;
        app->selected_network = 0;
        app->scroll_offset = 0;
        snprintf(app->last_message, sizeof(app->last_message), "Scanning Wi-Fi...");
    } else if(strncmp(line, "SCAN_COUNT ", 11) == 0) {
        app->network_total = (uint16_t)atoi(line + 11);
    } else if(strncmp(line, "AP ", 3) == 0) {
        c3_network_parse_ap(app, line);
    } else if(strcmp(line, "SCAN_END") == 0) {
        app->scanning = false;
        snprintf(app->last_message, sizeof(app->last_message), "SCAN COMPLETE");
    } else if(strncmp(line, "ERROR ", 6) == 0) {
        app->scanning = false;
        snprintf(app->last_message, sizeof(app->last_message), "ESP32 error");
    }

    furi_mutex_release(app->mutex);
}

static void c3_network_send_command(C3NetworkApp* app, const char* command) {
    if(!app->serial) return;

    char frame[12];
    const int length = snprintf(frame, sizeof(frame), "%s\n", command);
    if(length > 0 && (size_t)length < sizeof(frame)) {
        furi_hal_serial_tx(app->serial, (const uint8_t*)frame, (size_t)length);
    }
}

static void c3_network_draw_callback(Canvas* canvas, void* context) {
    C3NetworkApp* app = context;
    char line[48];

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "C3 NETWORK TOOLKIT");
    canvas_draw_line(canvas, 0, 13, 127, 13);
    canvas_set_font(canvas, FontSecondary);

    if(app->show_results) {
        if(app->show_details && app->network_count > 0) {
            const C3NetworkAp* ap = &app->networks[app->selected_network];
            canvas_draw_str(canvas, 2, 23, "ACCESS POINT DETAILS");
            snprintf(line, sizeof(line), "SSID: %.20s", ap->ssid);
            canvas_draw_str(canvas, 2, 33, line);
            snprintf(line, sizeof(line), "BSSID: %.17s", ap->bssid);
            canvas_draw_str(canvas, 2, 42, line);
            snprintf(line, sizeof(line), "RSSI: %d dBm  CH: %u", ap->rssi, ap->channel);
            canvas_draw_str(canvas, 2, 51, line);
            snprintf(line, sizeof(line), "SEC: %s", c3_network_auth_name(ap->auth));
            canvas_draw_str(canvas, 2, 60, line);
        } else {
            snprintf(
                line,
                sizeof(line),
                "Wi-Fi %u APs  %u/%u",
                (unsigned int)app->network_total,
                app->network_count ? (unsigned int)(app->selected_network + 1U) : 0U,
                (unsigned int)app->network_count);
            canvas_draw_str(canvas, 2, 23, line);

            for(uint8_t row = 0; row < VISIBLE_NETWORKS; row++) {
                const uint8_t index = (uint8_t)(app->scroll_offset + row);
                if(index < app->network_count) {
                    const C3NetworkAp* ap = &app->networks[index];
                    snprintf(
                        line,
                        sizeof(line),
                        "%c%.13s %4d",
                        index == app->selected_network ? '>' : ' ',
                        ap->ssid,
                        ap->rssi);
                    canvas_draw_str(canvas, 2, (uint8_t)(33 + (row * 9)), line);
                }
            }

            if(app->network_count == 0 && !app->scanning) {
                canvas_draw_str(canvas, 2, 42, "No networks found");
            } else if(app->scanning) {
                canvas_draw_str(canvas, 2, 60, "Scanning...");
            } else {
                canvas_draw_str(canvas, 2, 60, "OK: details  BACK: menu");
            }
        }
    } else {
        canvas_draw_str(canvas, 2, 23, app->serial ? "UART: 115200 READY" : "UART: BUSY");
        const char* items[] = {"PING", "STATUS", "SCAN WI-FI"};

        for(uint8_t i = 0; i < 3; i++) {
            snprintf(line, sizeof(line), "%c %s", app->menu_index == i ? '>' : ' ', items[i]);
            canvas_draw_str(canvas, 3, (uint8_t)(35 + (i * 9)), line);
        }

        canvas_draw_str(canvas, 2, 63, app->last_message);
    }

    furi_mutex_release(app->mutex);
}

static void c3_network_input_callback(InputEvent* event, void* context) {
    C3NetworkApp* app = context;
    if(event->type != InputTypeShort) return;

    bool update = false;

    furi_mutex_acquire(app->mutex, FuriWaitForever);

    if(event->key == InputKeyBack) {
        if(app->show_results && app->show_details) {
            app->show_details = false;
            update = true;
        } else if(app->show_results) {
            app->show_results = false;
            update = true;
        } else {
            app->running = false;
        }
    } else if(app->show_results && !app->show_details && event->key == InputKeyUp) {
        if(app->selected_network > 0) {
            app->selected_network--;
            if(app->selected_network < app->scroll_offset) {
                app->scroll_offset = app->selected_network;
            }
            update = true;
        }
    } else if(app->show_results && !app->show_details && event->key == InputKeyDown) {
        if(app->selected_network + 1U < app->network_count) {
            app->selected_network++;
            if(app->selected_network >= app->scroll_offset + VISIBLE_NETWORKS) {
                app->scroll_offset = (uint8_t)(app->selected_network - VISIBLE_NETWORKS + 1U);
            }
            update = true;
        }
    } else if(app->show_results && !app->show_details && event->key == InputKeyOk) {
        if(app->network_count > 0) {
            app->show_details = true;
            update = true;
        }
    } else if(!app->show_results && event->key == InputKeyUp) {
        app->menu_index = app->menu_index == 0 ? 2 : (uint8_t)(app->menu_index - 1);
        update = true;
    } else if(!app->show_results && event->key == InputKeyDown) {
        app->menu_index = (uint8_t)((app->menu_index + 1) % 3);
        update = true;
    } else if(!app->show_results && event->key == InputKeyOk) {
        const char* commands[] = {"PING", "STATUS", "SCAN"};

        if(app->serial) {
            snprintf(app->pending_command, sizeof(app->pending_command), "%s",
                     commands[app->menu_index]);
            app->command_pending = true;

            if(app->menu_index == 2) {
                app->show_results = true;
                app->show_details = false;
                app->scanning = true;
                app->network_total = 0;
                app->network_count = 0;
                app->selected_network = 0;
                app->scroll_offset = 0;
                snprintf(app->last_message, sizeof(app->last_message), "Scanning...");
            } else {
                snprintf(app->last_message, sizeof(app->last_message), "Command sent");
            }
        } else {
            snprintf(app->last_message, sizeof(app->last_message), "UART unavailable");
        }
        update = true;
    }

    furi_mutex_release(app->mutex);

    if(update) view_port_update(app->view_port);
}

static void c3_network_drain_uart(C3NetworkApp* app) {
    while(app->rx_tail != app->rx_head) {
        const uint8_t byte = app->rx_buffer[app->rx_tail];
        app->rx_tail = (uint16_t)((app->rx_tail + 1U) & RX_BUFFER_MASK);

        if(byte == '\r') continue;

        if(byte == '\n') {
            app->rx_line[app->rx_line_length] = '\0';
            if(app->rx_line_length > 0) {
                c3_network_process_line(app, app->rx_line);
                view_port_update(app->view_port);
            }
            app->rx_line_length = 0;
        } else if(app->rx_line_length < (RX_LINE_SIZE - 1U)) {
            app->rx_line[app->rx_line_length++] = (char)byte;
        } else {
            app->rx_line_length = 0;
        }
    }
}

int32_t c3_network_toolkit_app(void* p) {
    UNUSED(p);

    C3NetworkApp* app = malloc(sizeof(C3NetworkApp));
    furi_check(app);
    memset(app, 0, sizeof(C3NetworkApp));

    app->running = true;
    snprintf(app->last_message, sizeof(app->last_message), "Ready");
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, c3_network_draw_callback, app);
    view_port_input_callback_set(app->view_port, c3_network_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(app->serial) {
        furi_hal_serial_init(app->serial, UART_BAUD);
        furi_hal_serial_async_rx_start(
            app->serial, c3_network_uart_rx_callback, app, false);
        FURI_LOG_I(TAG, "UART initialized at %lu baud", (unsigned long)UART_BAUD);
    } else {
        snprintf(app->last_message, sizeof(app->last_message), "UART busy");
        FURI_LOG_W(TAG, "USART is already in use");
    }

    view_port_update(app->view_port);

    while(app->running) {
        char command[sizeof(app->pending_command)] = {0};
        bool send_command = false;

        furi_mutex_acquire(app->mutex, FuriWaitForever);
        if(app->command_pending) {
            snprintf(command, sizeof(command), "%s", app->pending_command);
            app->command_pending = false;
            send_command = true;
        }
        furi_mutex_release(app->mutex);

        if(send_command) c3_network_send_command(app, command);

        if(app->serial) c3_network_drain_uart(app);
        furi_delay_ms(20);
    }

    if(app->serial) {
        furi_hal_serial_async_rx_stop(app->serial);
        furi_hal_serial_control_release(app->serial);
    }

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
