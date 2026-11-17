# Flipper C3 Network Toolkit

Projeto experimental para integrar um ESP32-C3 Super Mini ao Flipper Zero por UART, com foco em diagnóstico e inventário passivo de redes e dispositivos em ambientes próprios ou autorizados.

## Estado do projeto

- Firmware ESP32-C3: USB CDC, UART, comandos e scanner Wi-Fi implementados e testados em hardware.
- Comunicação UART: PING/PONG, STATUS e SCAN testados com o Flipper Zero.
- Aplicativo nativo Flipper: primeira implementação em desenvolvimento; build automatizado via uFBT.
- BLE e ferramentas adicionais: planejados.

## Objetivos

- Scanner Wi-Fi 2,4 GHz: SSID, BSSID, canal, RSSI e tipo de autenticação.
- Protocolo serial simples entre ESP32-C3 e Flipper Zero.
- Aplicação nativa .fap para controlar as funções e apresentar resultados no display do Flipper.
- Diagnóstico de conectividade em redes autorizadas.
- Explorar BLE scan passivo em uma etapa posterior, respeitando as limitações de memória e rádio do C3.

## Hardware inicial

- Flipper Zero com firmware oficial.
- ESP32-C3 Super Mini com USB-C.
- UART cruzada:
  - Flipper pino 13 (TX) → ESP32-C3 GPIO20 (RX)
  - Flipper pino 14 (RX) ← ESP32-C3 GPIO21 (TX)
  - GND comum: Flipper pino 11 ↔ GND do ESP32-C3
- Durante a programação USB, não conecte simultaneamente o 3V3 do Flipper e o USB-C do ESP32-C3. Para a validação, alimente o C3 pelo USB-C e mantenha GND/TX/RX para UART.

Confira a pinagem da sua revisão de placa antes de energizar. GPIO20/21 são usados como UART RX/TX no firmware atual.

## Firmware ESP32-C3

O projeto usa PlatformIO com Arduino framework. Consulte firmware/README.md para compilar, gravar e testar.

## Aplicativo Flipper Zero

O aplicativo externo está em flipper_app/README.md. Ele usa o SDK oficial e uFBT para gerar um pacote .fap.

## Segurança e escopo

As funções iniciais são de descoberta e diagnóstico passivos. Use somente em redes e dispositivos próprios ou para os quais você tenha autorização. O projeto não implementa desautenticação, interferência, quebra de senha ou exploração de terceiros.

## Roadmap

Veja docs/ROADMAP.md.

## Licença

A licença ainda será definida antes de uma publicação pública estável.
