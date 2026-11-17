#include <Arduino.h>
#include <WiFi.h>
#include "protocol.h"

static String usbCommand;
static String uartCommand;
static bool scanRequested = false;
static unsigned long lastHeartbeat = 0;

static void sendBoth(const String &message) {
    Serial.println(message);
    Serial1.println(message);
}

static void printHelp() {
    sendBoth("INFO commands=HELP,PING,SCAN,STATUS");
}

static void scanWifi() {
    sendBoth("SCAN_BEGIN");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    const int count = WiFi.scanNetworks(false, true);
    if (count < 0) {
        sendBoth("ERROR wifi_scan_failed");
    } else {
        sendBoth("SCAN_COUNT " + String(count));
        for (int i = 0; i < count; ++i) {
            String line = "AP ssid=\"" + WiFi.SSID(i) +
                "\" bssid=" + WiFi.BSSIDstr(i) +
                " rssi=" + String(WiFi.RSSI(i)) +
                " channel=" + String(WiFi.channel(i)) +
                " auth=" + String(static_cast<int>(WiFi.encryptionType(i)));
            sendBoth(line);
            delay(5);
        }
    }
    WiFi.scanDelete();
    sendBoth("SCAN_END");
}

static void handleCommand(String command) {
    command.trim();
    command.toUpperCase();
    if (command.isEmpty()) return;

    if (command == "HELP") {
        printHelp();
    } else if (command == "PING") {
        sendBoth("PONG");
    } else if (command == "STATUS") {
        sendBoth("STATUS device=ESP32-C3 uart=115200 wifi=2.4GHz");
    } else if (command == "SCAN") {
        scanRequested = true;
    } else {
        sendBoth("ERROR unknown_command");
    }
}

static void readCommands(Stream &stream, String &buffer) {
    while (stream.available()) {
        const char c = static_cast<char>(stream.read());
        if (c == '\r' || c == '\n') {
            if (!buffer.isEmpty()) {
                handleCommand(buffer);
                buffer = "";
            }
        } else if (buffer.length() < protocol::MAX_COMMAND_LENGTH) {
            buffer += c;
        } else {
            buffer = "";
            sendBoth("ERROR command_too_long");
        }
    }
}

void setup() {
    Serial.begin(protocol::BAUD);
    Serial1.begin(protocol::BAUD, SERIAL_8N1,
                  protocol::UART_RX_PIN, protocol::UART_TX_PIN);
    delay(1200);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    sendBoth("READY Flipper-C3-Network-Toolkit version=0.1.0");
    sendBoth("INFO transport=UART baud=115200 rx=GPIO20 tx=GPIO21");
    printHelp();
}

void loop() {
    readCommands(Serial, usbCommand);
    readCommands(Serial1, uartCommand);

    if (scanRequested) {
        scanRequested = false;
        scanWifi();
    }

    if (millis() - lastHeartbeat >= 10000) {
        lastHeartbeat = millis();
        sendBoth("HEARTBEAT");
    }
    delay(2);
}
