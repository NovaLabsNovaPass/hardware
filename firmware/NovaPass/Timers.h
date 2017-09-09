
class Timers {
  public:
    uint16_t KHzCtr;
    volatile uint16_t ISRKHzFlags; // volatile b/c it gets set in ISR

    uint16_t OneHzCtr;
    const uint16_t OneHzInitVal = 1000;
    uint16_t OneHzFlags;

    uint16_t HundredHzCtr;
    const uint16_t HundredHzInitVal = 10;
    uint16_t HundredHzFlags;


    void Setup(void);
    void Chores(void);
    uint16_t GetKHzFlag(void);
    uint16_t GetHundredHzFlag(void);
    uint16_t GetOneHzFlag(void);

    uint16_t TimeChoresKHzFlag;

    inline void TimerIrupDisable(void) { SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk; }
    inline void TimerIrupEnable(void) { SysTick->CTRL  |= SysTick_CTRL_ENABLE_Msk; }

};

extern Timers timers;
