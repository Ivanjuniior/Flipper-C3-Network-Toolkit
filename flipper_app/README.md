# Aplicativo nativo para Flipper Zero

Esta pasta será implementada depois que o protocolo UART do ESP32-C3 estiver validado.

## Plano

1. Criar o aplicativo usando o SDK oficial do firmware Flipper Zero.
2. Registrar o aplicativo com `application.fam` e menus próprios.
3. Abrir UART nos pinos externos e negociar o protocolo do toolkit.
4. Implementar telas de status, scanner Wi-Fi e diagnóstico.
5. Testar compilação com o firmware oficial e documentar a instalação.

Não existe ainda um `application.fam` ou binário `.fap` nesta etapa: evitar publicar um esqueleto não compilável como se fosse um aplicativo pronto.
