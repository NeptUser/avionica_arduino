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
  * Realiza a leitura do MPU6050 (acelerômetro e giroscópio).
  * 
  * Atualiza os campos correspondentes na estrutura global `dadosVoo`.
  * Também registra um timestamp próprio para essa leitura.
  */
  void readData();

}

#endif // SENSORS_HPP