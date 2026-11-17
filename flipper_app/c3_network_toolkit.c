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
#define MAX_VISIBLE_NETWORKS 4U
#define SSID_SIZE 33U

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
    bool scanning;
    uint8_t menu_index;

    uint16_t network_total;
    uint8_t network_count;
    char network_ssids[MAX_VISIBLE_NETWORKS][SSID_SIZE];
    int16_t network_rssi[MAX_VISIBLE_NETWORKS];

    char last_message[32];
} C3NetworkApp;

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

static void c3_network_process_line(C3NetworkApp* app, const char* line) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    if(strcmp(line, "PONG") == 0) {
        snprintf(app->last_message, sizeof(app->last_message), "PONG received");
    } else if(strncmp(line, "STATUS ", 7) == 0) {
        snprintf(app->last_message, sizeof(app->last_message), "ESP32-C3 online");
    } else if(strcmp(line, "SCAN_BEGIN") == 0) {
        app->scanning = true;
        app->show_results = true;
        app->network_total = 0;
        app->network_count = 0;
        snprintf(app->last_message, sizeof(app->last_message), "Scanning Wi-Fi...");
    } else if(strncmp(line, "SCAN_COUNT ", 11) == 0) {
        app->network_total = (uint16_t)atoi(line + 11);
    } else if(strncmp(line, "AP ", 3) == 0) {
        const char* ssid_start = strstr(line, "ssid=\"");
        const char* rssi_start = strstr(line, "rssi=");

        if(ssid_start && rssi_start) {
            ssid_start += 6;
            const char* ssid_end = strchr(ssid_start, '\"');

            if(ssid_end && app->network_count < MAX_VISIBLE_NETWORKS) {
                const size_t length = (size_t)(ssid_end - ssid_start);
                char* destination = app->network_ssids[app->network_count];

                if(length == 0) {
                    snprintf(destination, SSID_SIZE, "<hidden>");
                } else {
                    const size_t copy_length = length < (SSID_SIZE - 1U) ?
                                                   length :
                                                   (SSID_SIZE - 1U);
                    memcpy(destination, ssid_start, copy_length);
                    destination[copy_length] = '\0';
                }

                app->network_rssi[app->network_count] = (int16_t)atoi(rssi_start + 5);
                app->network_count++;
            }
        }
    } else if(strcmp(line, "SCAN_END") == 0) {
        app->scanning = false;
        snprintf(app->last_message, sizeof(app->last_message), "SCAN COMPLETE");
    } else if(strncmp(line, "ERROR ", 6) == 0) {
        snprintf(app->last_message, sizeof(app->last_message), "ESP32 error");
        app->scanning = false;
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
    canvas_set_font(canvas, CanvasFontPrimary);
    canvas_draw_str(canvas, 2, 10, "C3 NETWORK TOOLKIT");
    canvas_draw_line(canvas, 0, 13, 127, 13);

    canvas_set_font(canvas, CanvasFontSecondary);

    if(app->show_results) {
        canvas_draw_str(canvas, 2, 23, app->scanning ? "Scanning Wi-Fi..." : "Wi-Fi results");

        for(uint8_t i = 0; i < MAX_VISIBLE_NETWORKS; i++) {
            if(i < app->network_count) {
                snprintf(
                    line,
                    sizeof(line),
                    "%s %d dBm",
                    app->network_ssids[i],
                    app->network_rssi[i]);
                canvas_draw_str(canvas, 2, (uint8_t)(34 + (i * 8)), line);
            }
        }

        if(!app->scanning && app->network_total == 0 && app->network_count == 0) {
            canvas_draw_str(canvas, 2, 39, "No networks found");
        }

        snprintf(
            line,
            sizeof(line),
            "%u found  |  BACK: menu",
            (unsigned int)app->network_total);
        canvas_draw_str(canvas, 2, 63, line);
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
        if(app->show_results) {
            app->show_results = false;
            update = true;
        } else {
            app->running = false;
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
                app->scanning = true;
                app->network_total = 0;
                app->network_count = 0;
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
