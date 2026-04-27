// telemetry.hpp
// Declara as funções de telemetria via módulo LoRa E32-915MHz.
//
// O módulo E32 opera via UART (SoftwareSerial) e é controlado
// pelos pinos M0, M1 (modo de operação) e AUX (status).
//
// O pino AUX é monitorado via interrupção externa INT0 (EIMSK/EICRA),
// evitando polling e liberando o loop principal para outras tarefas.
//
// Modos de operação (M0, M1):
//   0,0 → Normal  | 0,1 → Wake-up
//   1,0 → Economy | 1,1 → Sleep/Config

#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

namespace Telemetry {
  /**
  * Inicializa a SoftwareSerial, configura os pinos M0 e M1
  * e AUX via registradores DDRx/PORTx.
  * Deve ser chamada uma vez no setup() do .ino.
  * @return true se o módulo respondeu (AUX HIGH após init).
  */
  bool setup();

  /**
  * Monta um PacoteTelemetria a partir do dadosVoo global
  * e o serializa para o módulo E32 via SoftwareSerial.
  * Antes de transmitir, verifica via polling (PIND) se o
  * pino AUX está HIGH — indicando que o módulo está livre.
  */
  void sendPacket();

  /**
  * Lê e processa um pacote recebido pelo módulo E32.
  * Recepção de pacotes não implementada via interrupção.
  */
  void rcvPacket();
}

#endif // TELEMETRY_HPP