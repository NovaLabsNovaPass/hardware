#include <Arduino.h>
#include "Timers.h"


void Timers::Setup(void) {
  // called once from sketch Setup()
  OneHzCtr          = OneHzInitVal;
  HundredHzCtr      = HundredHzInitVal;
  TimeChoresKHzFlag = GetKHzFlag();

}


void Timers::Chores(void) {

  TimerIrupDisable();

  // Check if 1khz flag was set in timer isr
  if (ISRKHzFlags & TimeChoresKHzFlag) {
    ISRKHzFlags &= ~TimeChoresKHzFlag; 

    TimerIrupEnable();

    if (!(--OneHzCtr)) {
      OneHzCtr = OneHzInitVal;
      OneHzFlags = 0b1111111111111111;
    }
    if (!(--HundredHzCtr)) {
      HundredHzCtr = HundredHzInitVal;
      /*
      if (HundredHzFlags & 1)
          SerialUSB.print("TimerOvrn");
      */

      HundredHzFlags = 0b1111111111111111;
    }
    return;
  }
  TimerIrupEnable();
}




uint16_t Timers::GetKHzFlag(void) {
  //
  // allocates timer flags *Chores() functions 
  //
  static uint16_t nextflag = 0;
  uint16_t ret = (1 << nextflag);
  nextflag++;
  return(ret);
}

uint16_t Timers::GetHundredHzFlag(void) {
  //
  // allocates timer flags *Chores() functions 
  //
  static uint16_t nextflag = 0;
  uint16_t ret = (1 << nextflag);
  nextflag++;
  SerialUSB.print("GetHundredHzFlag()");
  return(ret);
}

uint16_t Timers::GetOneHzFlag(void) {
  //
  // allocates timer flags *Chores() functions 
  //
  static uint16_t nextflag = 0;
  uint16_t ret = (1 << nextflag);
  nextflag++;
  return(ret);
}


// Arduino run-time already has a 1ms interrupt setup..
// Apps can get called using sysTickHook() 
#ifdef __cplusplus
extern "C" {
#endif
  int sysTickHook(void) {
    //
    // Gets called every 1 msec or so.. 
    //
    // ISR Sets a whole wordsworth of flags
    // Pieces of the programmed loop get to unset
    // individual ones..
    //
    timers.ISRKHzFlags = 0b1111111111111111;

    return 0; // 0 says yes run the default sysTick function after this.
  }
#ifdef __cplusplus
}
#endif


