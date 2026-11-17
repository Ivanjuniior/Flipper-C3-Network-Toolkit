# Protocolo serial v0

- Transporte: UART 3,3 V, 115200 baud, 8N1.
- Uma mensagem ASCII por linha, terminada em LF ou CRLF.
- Comandos: `HELP`, `PING`, `STATUS`, `SCAN`.
- Respostas: `READY`, `INFO`, `PONG`, `STATUS`, `SCAN_BEGIN`, `SCAN_COUNT`, `AP`, `SCAN_END`, `HEARTBEAT`, `ERROR`.
- A versão inicial é textual para facilitar depuração pelo USB Serial Monitor.

O formato `AP` é provisório e será formalizado com escaping adequado para SSIDs que contenham aspas ou caracteres especiais antes da integração final com o app Flipper.
