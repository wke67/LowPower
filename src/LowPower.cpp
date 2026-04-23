/*
Low power hibernation for AVR DxCore, AVR xmega.
These chips have an RTC clock running at 32kHz.
When the RTC is used as Arduino millis() timer, this code
uses the running RTC to implement a delay() equivalent that
sends the chip into low power Sleep_Standby_Mode during delay.

In this case, the code relies on the RTC interrupt routine
in DxCore/../cores/.../wiring.c which needs to store the
interrupt flags to __rtc_intflags.

If the RTC Timer is not used, the RTC XTAL oscillator is
initialized and the RTC interrupt is handled here.
The millis() function is updated but returned time is not
correct to the ms.

*/

#include <Arduino.h>
#include <LowPower.h>
#include <avr/sleep.h>
#include <pins_arduino.h>

#ifndef CLKCTRL_LPMODE_bm
  #define CLKCTRL_LPMODE_bm 0
#endif

#define RTC_MAX 0xffff


char __rtc_intflags __attribute__((weak)) = 33;

#ifndef MILLIS_USE_TIMERRTC

static uint32_t timer_overflow_count;

ISR(RTC_CNT_vect) {
  // if RTC is used as timer, we only increment the overflow count
  // Overflow count isn't used for TCB's
  // both are needed for TCA and TCD.
  if (RTC.INTFLAGS & RTC_OVF_bm) {
    timer_overflow_count++;
  }
  __rtc_intflags = RTC.INTFLAGS;
  RTC.INTFLAGS = RTC_OVF_bm | RTC_CMP_bm; // clear flag
}

LowPowerClass::LowPowerClass(uint8_t mode) {

  uint8_t status=0;

  _mode = mode;
  RTC_CTRLA = RTC_RUNSTDBY_bm | RTC_PRESCALER_DIV32_gc | RTC_RTCEN_bm;
  switch (_mode) {

    case LOWPOWER_XTAL:
      #ifdef LOWPOWER_XTAL
      // set up RTC with 32kHz crystal running at 1024 Hz
      _PROTECTED_WRITE(CLKCTRL_XOSC32KCTRLA, (CLKCTRL_RUNSTDBY_bm | CLKCTRL_ENABLE_bm | CLKCTRL_CSUT_16K_gc | CLKCTRL_LPMODE_bm)) ;
      RTC_CLKSEL = RTC_CLKSEL_XTAL32K_gc; // RTC_CLKSEL_XOSC32K_gc == 2
      status = timeout(CLKCTRL_XOSC32KS_bm);
      #endif
      break;

    case LOWPOWER_EXT:
      RTC_CLKSEL = RTC_CLKSEL_EXTCLK_gc;
      status = timeout(CLKCTRL_EXTS_bm);
      break;

    case LOWPOWER_INT:
      RTC_CLKSEL = RTC_CLKSEL_OSC32K_gc;
      status = timeout(CLKCTRL_OSC32KS_bm);
      break;

    default:
      status = 0;
      break;
  }

  if (status == 0) {
    return;
  }
  // set up RTC period
  while (RTC.STATUS && RTC_PERBUSY_bm);
  RTC.PER = RTC_MAX;
  while (RTC.STATUS && RTC_CNTBUSY_bm);
  RTC.CNT = 0;
  timer_overflow_count = 0;

  // RTC Interrupt enable
  RTC_INTCTRL = RTC_OVF_bm;

}


uint8_t LowPowerClass::timeout(uint8_t mode) {

  for (int i = 0; i < 2000; i++) {
    if (CLKCTRL.MCLKSTATUS & mode) {
      return mode;
    }
    delayMicroseconds(1000);
  }
  return 0;
}


// from wiring.c
unsigned long LowPowerClass::millis() {
  // return timer_overflow_count; // for debugging timekeeping issues where these variables are out of scope from the sketch
  unsigned long m;
  // disable interrupts while we read timer_millis or we might get an
  // inconsistent value (e.g. in the middle of a write to timer_millis)
  uint8_t oldSREG = SREG;
  cli();
  uint16_t rtccount = RTC.CNT;
  m = timer_overflow_count;
  if (RTC.INTFLAGS & RTC_OVF_bm) {
    /* There has just been an overflow that hasn't been accounted for by the interrupt. Check if the high bit of counter is set.
      * We just basically need to make sure that it didn't JUST roll over at the last couple of clocks. But this merthod is
      * implemented very efficiently (just an sbrs) so it is more efficient than other approaches. If user code is leaving
      * interrupts off nearly 30 seconds, they shouldn't be surprised. */
    if (!(rtccount & 0x8000)) {
      m++;
    }
  }
  SREG = oldSREG;
  m = (m << 16);
  m += rtccount;
  uint8_t round = ((m & 0x3f) + ((m & 0x7f) >> 1) + 32) >> 6;
  m = m - (m >> 6) - (m >> 7) - round;  // * 1000/1024 -> * ( 1 - 24/1024 ) -> * ( 1 - 1/64 - 1/128 )
  return m;
}

void LowPowerClass::set_millis(uint32_t newmillis) {
  // millis = 1000/1024*(timer_overflow_count << 16 + RTC.CNT)
  uint8_t oldSREG = SREG; // save SREG
  newmillis = ((newmillis / 125) << 7) + (((newmillis % 125) << 7) + 62) / 125;  // *1024/1000
  cli();                  // interrupts off
  timer_overflow_count = newmillis >> 16;
  while (RTC.STATUS & RTC_CNTBUSY_bm); // wait if RTC busy
  RTC.CNT = newmillis & 0xFFFF;
  SREG = oldSREG; // re-enable interrupts if we killed them,
}

void LowPowerClass::restart_millis() {
  set_millis(0);
}

#else

LowPowerClass::LowPowerClass(uint8_t mode) {
  #if defined(MILLIS_USE_TIMERRTC_XTAL)
  _mode = LOWPOWER_XTAL;
  #elif defined(MILLIS_USE_TIMERRTC_OSC)
  _mode = LOWPOWER_INT;
  #else
  _mode= LOWPOWER_EXT;
  #endif
}

unsigned long LowPowerClass::millis() {
  return ::millis();
}
void LowPowerClass::set_millis(uint32_t newmillis) {
  ::set_millis(newmillis);
}
void LowPowerClass::restart_millis() {
  ::restart_millis();
};

#endif

uint8_t LowPowerClass::status() {
  return CLKCTRL.MCLKSTATUS & _mode;
}

void LowPowerClass::sleep(unsigned long dly) {
  volatile unsigned int cntr, cnt;

  dly = dly + (dly >> 6) + (dly >> 7) + (dly >> 11) + (dly >> 14); // ms -> 1024Hz
  do {
    cnt = (dly > RTC_MAX) ? RTC_MAX : dly;
    dly =  dly - cnt;
    while (RTC.STATUS && RTC_CNTBUSY_bm); // wait for RTC.CNT sync
    cntr = RTC.CNT;
    while (RTC.STATUS && RTC_CMPBUSY_bm); // wait for RTC.CNT sync
    RTC.CMP = (cntr + cnt - 1) & RTC_MAX;
    RTC.INTCTRL |= RTC_CMP_bm; //enable CMP Interrupt RTC_CMP

    set_sleep_mode(SLEEP_MODE_STANDBY);
    sleep_enable();
    sleep_cpu();
    if (__rtc_intflags == RTC_OVF_bm) {
      sleep_cpu();
    }
    sleep_disable();
    while (RTC.STATUS && RTC_CNTBUSY_bm) {};   // required for millis() to work!

    RTC.INTCTRL &= ~RTC_CMP_bm;
  } while (dly);

  #if !defined(MILLIS_USE_TIMERRTC) && !defined(MILLIS_USE_TIMERNONE)
  ::set_millis(millis());
  #endif
}
