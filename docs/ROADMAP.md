# Roadmap

## Fase 0 — Estrutura e validação básica
- [x] Criar estrutura inicial do repositório.
- [x] Definir configuração PlatformIO para ESP32-C3 e USB CDC.
- [x] Implementar protocolo serial textual mínimo.
- [ ] Validar build e upload em hardware real.
- [ ] Confirmar saída USB CDC e UART com Flipper.

## Fase 1 — Wi-Fi
- [x] Implementar comando SCAN e inventário passivo de APs.
- [ ] Validar SSID com caracteres especiais e redes ocultas.
- [ ] Melhorar parser e formato de resposta.
- [ ] Testar estabilidade e uso de memória.

## Fase 2 — Aplicativo Flipper
- [ ] Implementar app nativo `.fap` usando SDK oficial.
- [ ] Menu e tela de status.
- [ ] Controle de varredura e apresentação dos resultados.
- [ ] Testar no firmware oficial.

## Fase 3 — Diagnóstico
- [ ] Cliente Wi-Fi para rede própria/autorizada.
- [ ] Diagnóstico de IP, gateway e DNS.
- [ ] Testes de conectividade.

## Fase 4 — BLE e publicação
- [ ] Avaliar BLE scan passivo no C3.
- [ ] Documentar limitações de hardware.
- [ ] Testes, screenshots e instruções de instalação.
- [ ] Revisar licença e requisitos de submissão do Flipper Lab.
