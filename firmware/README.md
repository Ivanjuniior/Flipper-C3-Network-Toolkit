# Firmware ESP32-C3

## Requisitos

- VS Code + PlatformIO (recomendado), ou PlatformIO Core.
- ESP32-C3 Super Mini conectado por USB-C.

## Compilar e gravar

Na pasta `firmware/`:

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```

A porta pode ser selecionada explicitamente com `--upload-port /dev/ttyACM0` e `--port /dev/ttyACM0`, se esse for o dispositivo correto.

## Teste inicial

Com o C3 ligado por USB, o monitor deve mostrar a inicialização. Digite `SCAN` e envie uma nova linha para iniciar uma varredura Wi-Fi. O resultado também é transmitido pela UART (GPIO20 RX / GPIO21 TX) a 115200 baud.

## Alimentação e UART

Durante a gravação USB, remova o fio 3V3 vindo do Flipper. Não alimente a placa simultaneamente por fontes independentes sem confirmar o circuito e evitar retorno de corrente. Para UART, mantenha GND comum e TX/RX cruzados.
