# C3 Network Toolkit — Flipper app

Aplicativo externo (.fap) para controlar o ESP32-C3 Super Mini conectado ao Flipper Zero pela UART de expansão.

## Recursos

- Interface nativa com navegação pelos botões.
- UART USART a 115200 baud.
- Comandos PING, STATUS e SCAN.
- Recepção de SSID, BSSID, RSSI, canal e tipo de autenticação.
- Lista navegável de até 20 pontos de acesso por varredura.
- Tela de detalhes para cada ponto de acesso.
- Navegação pela lista: cima/baixo; OK abre detalhes; BACK retorna.
- A implementação de aquisição e liberação da UART mantém o fluxo da versão previamente testada.

## Hardware

- Flipper TX (pino 13) → ESP32-C3 GPIO20 (RX).
- Flipper RX (pino 14) ← ESP32-C3 GPIO21 (TX).
- GND comum.
- Não conecte a saída 3V3 do Flipper enquanto o ESP32-C3 estiver alimentado pela USB.

Confirme a pinagem da sua revisão de placa antes de energizar.

## Build com uFBT

Compile a partir desta pasta:

    cd flipper_app
    ufbt

O pacote será gerado em dist/c3_network_toolkit.fap.

Para enviar e iniciar no Flipper conectado por USB:

    ufbt launch

## Protocolo esperado

O firmware responde com linhas terminadas em newline. Para cada ponto de acesso:

    AP ssid="..." bssid=AA:BB:CC:DD:EE:FF rssi=-55 channel=6 auth=3

O parser usa os campos SSID, BSSID, RSSI, canal e código de autenticação enviados pelo firmware.

## Limitações desta versão

- A lista armazena até 20 redes por varredura; a contagem total recebida do ESP32 continua sendo exibida.
- A lista depende do formato textual documentado acima.
- BLE e diagnósticos adicionais de conectividade serão implementados em etapas posteriores.
