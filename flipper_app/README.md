# C3 Network Toolkit — Flipper app

Aplicativo externo (.fap) para controlar o ESP32-C3 Super Mini conectado ao Flipper Zero pela UART de expansão.

## Primeira versão

- Interface nativa com navegação pelos botões.
- UART USART a 115200 baud.
- Comandos PING, STATUS e SCAN.
- Exibição de até quatro SSIDs e seus RSSI.
- Leitura do protocolo textual por linhas.

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

## Controles

- Cima/baixo: selecionar PING, STATUS ou SCAN WI-FI.
- OK: executar a opção selecionada.
- BACK: voltar dos resultados ou sair do aplicativo.

## Limitações desta versão

- A tela de scanner exibe até quatro redes encontradas, mantendo a contagem total recebida.
- O protocolo atual é textual e orientado a linhas.
- BLE, diagnósticos de rede e funções adicionais serão adicionados em versões posteriores.
