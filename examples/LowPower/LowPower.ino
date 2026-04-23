/*
 * LowPower Example
 * timed sleep for Microchip megaaavr devices, e.g. avr64dd28 attiny3217
 * compare millis() time with Arduino IDE output time
 */

#include <LowPower.h>

LowPowerClass LP(LOWPOWER_INT);

void setup() {
  int mode;

  Serial.begin(115200);
  Serial.printf("start: rtc ");
  for (uint8_t i = 0; i < 1200; i++) {        // wait for clock startup
    mode = LP.status();
    if ( mode ) break;
    delay(1);
  }
  Serial.printf("%s\n", (mode==LOWPOWER_XTAL) ? "xtal" \
    : (mode==LOWPOWER_INT) ? "osc"  \
    : (mode==LOWPOWER_EXT) ? "xosc" \
    : "off");

  Serial.printf("press enter to start\n");   // synchronize
  while(Serial.available() == 0);
  while(Serial.available() ) { Serial.read(); delay(10); }
  Serial.flush();
  LP.set_millis(0);
}

uint32_t t, tz=0, st=1001;
void loop() {
  LP.sleep(st);
  Serial.printf("%2.2d:%2.2d.%3.3d %ld\n", int(t/1000)/60, int(t/1000)%60, int(t%1000), st );
  Serial.flush();
  tz += 1000;
  t = millis();
  while (t >= tz + 1000) tz += 1000;
  st=tz + 1000 - t;
}
