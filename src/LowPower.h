#ifndef LOWPOWER_H
#define LOWPOWER_H

#ifdef CLKCTRL_XOSC32KS_bm
#define LOWPOWER_XTAL CLKCTRL_XOSC32KS_bm
#endif
#define LOWPOWER_EXT  CLKCTRL_EXTS_bm
#define LOWPOWER_INT  CLKCTRL_OSC32KS_bm

class LowPowerClass {

    uint8_t timeout(uint8_t clk);
    uint8_t _status;

  public:
    LowPowerClass(uint8_t mode = LOWPOWER_EXT);
    uint8_t status() {
      return _status;
    }
    void sleep(unsigned long time);
    unsigned long millis();
    void set_millis(uint32_t newmillis);
    void restart_millis();

};

#endif
