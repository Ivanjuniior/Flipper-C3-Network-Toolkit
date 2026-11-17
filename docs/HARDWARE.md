# Hardware e conexões

## Montagem registrada

| ESP32-C3 Super Mini | Flipper Zero | Sinal |
|---|---|---|
| GND | pino 11 | GND |
| GPIO20 / RX | pino 13 / TX | UART do Flipper para o C3 |
| GPIO21 / TX | pino 14 / RX | UART do C3 para o Flipper |
| 3V3 | pino 9 | Alimentação usada inicialmente; não manter durante programação USB sem validação |

## Alimentação recomendada durante desenvolvimento

Alimente o C3 pelo USB-C durante gravação e depuração. Remova a ligação 3V3 ao Flipper para evitar duas fontes em paralelo. Para comunicação UART, mantenha GND comum e os sinais TX/RX cruzados. Antes de operação autônoma, validar consumo e capacidade da fonte de 3,3 V.

## Observação

Confirme a serigrafia e a revisão exata da placa. A numeração dos pinos do conector do Flipper é a numeração física da documentação oficial, não o número GPIO do ESP32.
