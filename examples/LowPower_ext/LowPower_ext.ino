/*
 * LowPower Example
 * timed sleep for Microchip megaaavr devices, e.g. avr64dd28 attiny3217
 */

#include <LowPower.h>

#define  LPCLOCK LOWPOWER_EXT
LowPowerClass LP(LPCLOCK);

void setup() {
  Serial.begin(115200);
  if (LP.status() != LPCLOCK) {
    Serial.printf("something went wrong: clock status %2.2x != %2.2x\n", LP.status(), LPCLOCK);
  }
  Serial.printf("start: clock status %2.2x\n", LP.status());
  while (LP.millis() < 3) ;    // wait for Clock startup
  LP.set_millis(100);
  Serial.printf("time: %ld\n", LP.millis());
  LP.restart_millis();
  Serial.printf("time: %ld\n", LP.millis());
  Serial.flush(); // flush Serial before sleep()
}

void loop() {
  LP.sleep(1000);
  Serial.printf("time: %ld\n", LP.millis());
  Serial.flush();
}
