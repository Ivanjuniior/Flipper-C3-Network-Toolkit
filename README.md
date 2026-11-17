# Flipper C3 Network Toolkit

Projeto experimental para integrar um **ESP32-C3 Super Mini** ao **Flipper Zero** por UART, com foco em diagnóstico e inventário passivo de redes e dispositivos em ambientes próprios ou autorizados.

> **Estado:** início do desenvolvimento. O firmware ESP32-C3 está em fase de validação. A aplicação nativa do Flipper ainda não foi implementada.

## Objetivos

- Scanner Wi-Fi 2,4 GHz: SSID, BSSID, canal, RSSI e tipo de autenticação.
- Protocolo serial simples entre ESP32-C3 e Flipper Zero.
- Aplicação nativa para controlar as funções e apresentar resultados no display do Flipper.
- Diagnóstico de conectividade em redes autorizadas.
- Explorar BLE scan passivo em uma etapa posterior, respeitando as limitações de memória e rádio do C3.

## Hardware inicial

- Flipper Zero com firmware oficial.
- ESP32-C3 Super Mini com USB-C.
- UART cruzada:
  - Flipper pino 13 (TX) → ESP32-C3 GPIO20 (RX)
  - Flipper pino 14 (RX) ← ESP32-C3 GPIO21 (TX)
  - GND comum: Flipper pino 11 ↔ GND do ESP32-C3
- A alimentação deve ser definida e testada separadamente. Durante a programação USB, **não conecte simultaneamente o 3V3 do Flipper e o USB-C do ESP32-C3**. Para a primeira validação, alimente o C3 pelo USB-C e mantenha GND/TX/RX para UART.

Confira a pinagem da sua revisão de placa antes de energizar. GPIO20/21 são usados como UART0 RX/TX em configurações comuns do ESP32-C3, mas o mapeamento pode variar conforme placa e configuração.

## Firmware ESP32-C3

O projeto usa PlatformIO com Arduino framework. Consulte [`firmware/README.md`](firmware/README.md) para compilar, gravar e testar.

## Aplicativo Flipper Zero

O diretório [`flipper_app/`](flipper_app/README.md) documenta a implementação planejada. O aplicativo nativo será desenvolvido usando o SDK e o formato `.fam` do firmware oficial do Flipper; não há binário pronto nesta etapa.

## Segurança e escopo

As funções iniciais são de descoberta e diagnóstico passivos. Use somente em redes e dispositivos próprios ou para os quais você tenha autorização. Não há recursos de desautenticação, interferência, quebra de senha ou exploração de terceiros neste projeto.

## Roadmap

Veja [`docs/ROADMAP.md`](docs/ROADMAP.md).

## Licença

A licença ainda será definida antes de uma publicação pública estável.
