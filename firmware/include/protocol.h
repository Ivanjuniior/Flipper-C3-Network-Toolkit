#pragma once

// Protocolo textual inicial, uma mensagem por linha.
// Comandos recebidos: HELP, PING, SCAN, STATUS.
// Eventos enviados: READY, PONG, SCAN_BEGIN, AP, SCAN_END, ERROR.
namespace protocol {
constexpr unsigned long BAUD = 115200;
constexpr int UART_RX_PIN = 20; // ESP32-C3 RX <- Flipper TX (pino 13)
constexpr int UART_TX_PIN = 21; // ESP32-C3 TX -> Flipper RX (pino 14)
constexpr unsigned long MAX_COMMAND_LENGTH = 64;
}
