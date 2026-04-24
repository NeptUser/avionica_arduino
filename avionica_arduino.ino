/* libs:
Adafruit MPU6050
Adafruid BMP280
EByte LoRa E32
SPIMemory
*/

#include "globals.hpp"
#include "storage.hpp"
#include "telemetry.hpp"
#include "sensors.hpp"

void setup() {
  Storage::setup();
  Telemetry::setup();
  Sensors::setup();
}

void loop() {
  // todo: control loop (using state machine?)
}