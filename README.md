# Low Power Sleep for AVR DxCore

Low power timed-sleep() for Microchip AVR chips which include the RTC module and run on Arduino DxCore, e.g. avr64dd32 or MegaTinyCore e.g. ATtiny1604.

The RTC module can run with internal, external or on-chip 32kHz Crystal oscillator clocks.
The chip can be powered down to 'Standby' consuming ~1uA. In this mode, almost all 
peripherals are at rest including timers. The time-keeping can be done by the RTC Clock, this
can be available from the Arduino IDE millis() timer menu.

When the RTC is used as Arduino millis() Timer, this code uses the running RTC to implement
a timed-sleep, which sends the chip into low power Sleep_Standby_Mode during delay.
In this case, the code relies on the RTC interrupt routine in DxCore/../cores/.../wiring.c which
needs to store the interrupt flags to __rtc_intflags. (DxCore update required!)

If the RTC Timer is not used by the DxCore platforem, the RTC clock source can be selected
in the class instantiation.

The library provides:
- `LowPower.h`
- `LowPowerClass(clock_source=LOWPOWER_INT)` others: LOWPOWER_XTAL, LOWPOWER_EXT
- `LowPowerClass::status()` returns clock_source, zero on error
- `void LowPowerClass::sleep(unsigned long time)`
- `unsigned long LowPowerClass::millis()`
- `void LowPowerClass::set_millis(uint32_t newmillis)`
- `void LowPowerClass::restart_millis()`

In case the RTC Timer is selected in the Arduino IDE the millis() functions will call the corresponding platform mills() functions.
