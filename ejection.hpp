// Ejection.hpp
// Declara as funções de atuação para ejeção do paraquedas.
//
// O sistema suporta acionamento via servo-motor (PWM) ou carga
// pirotécnica chaveada por MOSFET de potência.
//
// O acionamento via MOSFET (GND switching) permite chavear a corrente
// da carga pirotécnica, utilizando o pino digital apenas para controlar
// o Gate do transistor.
//
// Modos de operação:
//   SERVO → Destravamento mecânico (servo.h)
//   PYRO  → Ignição pirotécnica (Digital Out)

#ifndef EJECTION_HPP
#define EJECTION_HPP

#include "globals.hpp"

namespace Ejection {

  /**
  * Inicializa o sistema de ejeção.
  *
  * Define o hardware e o pino de controle a partir dos macros em globals.hpp.
  *
  * - Modo SERVO: inicializa a biblioteca servo.h (uso de timers internos)
  * - Modo PYRO: configura diretamente os registradores de I/O
  *
  * Deve ser chamada uma única vez no setup().
  */
  void setup();

  /**
  * Executa a sequência de ejeção conforme o modo pré-configurado.
  *
  * Modo SERVO:
  *   - Movimento 0° ↔ 90°
  *   - Repetido 3 vezes para garantir destravamento mecânico
  *
  * Modo PYRO:
  *   - 3 pulsos de 500ms em nível HIGH
  *   - Intervalos de ~500ms em LOW entre pulsos
  *   - Implementação não-bloqueante via millis()
  *
  * @return true quando a sequência foi completamente executada.
  *         (não garante sucesso físico da ejeção)
  */
  bool ejectionEvent();

}

#endif // EJECTION_HPP