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

## Hardware

- Flipper TX (pino 13) → ESP32-C3 GPIO20 (RX).
- Flipper RX (pino 14) ← ESP32-C3 GPIO21 (TX).
- GND comum.
- Não conecte a saída 3V3 do Flipper enquanto o ESP32-C3 estiver alimentado pela USB.

Confirme a pinagem da sua revisão de placa antes de energizar.

## Build com uFBT

Instale o uFBT oficial:

    python3 -m pip install --upgrade ufbt

Compile a partir desta pasta:

    cd flipper_app
    ufbt

O pacote será gerado em dist/c3_network_toolkit.fap.

Para enviar e iniciar no Flipper conectado por USB:

    ufbt launch

O comando ufbt launch requer o Flipper conectado por USB e uma versão de firmware compatível com o SDK instalado pelo uFBT.

## Limitações desta versão

- A lista armazena até 20 redes por varredura; a contagem total recebida do ESP32 continua sendo exibida.
- O protocolo atual é textual e orientado a linhas. O parser atual espera SSID entre aspas e os campos bssid, rssi, channel e auth.
- O tipo de autenticação é apresentado a partir do código numérico enviado pelo Arduino Wi-Fi API.
- BLE e diagnósticos de conectividade serão adicionados em etapas posteriores.
