# Low Power Sleep for AVR DxCore

Low power timed-sleep() for Microchip AVR (megaavr) running on Arduino DxCore platform.
Supports MegaAVR and MegaTinyAVR e.g. avr32dd64, attiny1604 chips which include the RTC module.

The RTC module can run with internal, external or on-chip 32kHz Crystal oscillator clocks.
The chip can be powered down to 'Standby' consuming ~1uA. In this mode, almost all 
peripherals are at rest including timers. The time-keeping can be done by the RTC Clock, this
can be available from the Arduino IDE millis() timer menu.

When the RTC is used as Arduino millis() Timer, this code uses the running RTC to implement
a timed-sleep, which sends the chip into low power Sleep_Standby_Mode during delay.
In this case, the code relies on the RTC interrupt routine in DxCore/../cores/.../wiring.c which
needs to store the interrupt flags to __rtc_intflags. (DxCore update required!)

If the RTC Timer is not used by the DxCore platforem, the RTC clock source can be selected
in the class instanciation.

The library provides:
- `LowPower.h`
- `LowPowerClass(clock_source)` e.g. LOWPOWER_XTAL
- `LowPowerClass::status()` returns clock_source, zero on error
- `void LowPowerClass::sleep(unsigned long time)`
- `unsigned long LowPowerClass::millis()`
- `void LowPowerClass::set_millis(uint32_t newmillis)`
- `void LowPowerClass::restart_millis()`

In case the RTC Timer is selected in the Arduino IDE the millis() functions will call the corresponding platform mills() functions.
