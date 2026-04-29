#ifndef SENSORS_HPP
#define SENSORS_HPP

namespace Sensor {
  /**
  * Inicializa o MPU-6050 e o BMP280 via I²C.
  * Deve ser chamada uma vez no setup() do .ino.
  * @return true se ambos os sensores responderam corretamente.
  */
  bool setup();

  /**
  * Realiza a leitura completa dos sensores MPU6050 e BMP280.
  *
  * Atualiza os campos correspondentes na estrutura global `dadosVoo`.
  * Também registra timestamps próprios para cada sensor e um
  * timestamp geral de leitura.
  */
  void readData();

  /**
  * Realiza a leitura do MPU6050 (acelerômetro e giroscópio).
  * 
  * Atualiza os campos correspondentes na estrutura global `dadosVoo`.
  * Também registra um timestamp próprio para essa leitura.
  * 
  * Esperado ser chamado em alta frequência (ex: 50–200 Hz),
  * normalmente acionado por interrupção ou polling rápido.
  */
  void readMPU();

  /**
  * Realiza a leitura do BMP280 (pressão, temperatura e altitude).
  * 
  * Atualiza os campos correspondentes na estrutura global `dadosVoo`.
  * Também registra um timestamp próprio para essa leitura.
  * 
  * Esperado ser chamado em baixa frequência (ex: 5–20 Hz),
  * controlado por lógica temporal (millis) na avionica central.
  */
  void readBMP();
}

#endif // SENSORS_HPP