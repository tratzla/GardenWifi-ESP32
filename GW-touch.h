#ifndef GW_TOUCH
#define GW_TOUCH

#include <Arduino.h>
#include "GW-sleep.h"

#define TOUCH7_AKA_GPIO27 27
#define TOUCH_THRESHOLD 40 /* 40 for Arduino(150 for ESP) Larger is more sensetive */

bool pin27Touched = true;

void touch_callback() {
  // Serial.println("! OH! You touched me on the GPIO 27! <touch_callback>");
  //   manScrollNext = true; 
  pin27Touched = true;
}

void checkTouchInput() {
  if (!pin27Touched) return;  
  pin27Touched = false;
  
  naptime = millis() + TIME_TO_WAKE_MS;

  if(autoScroll && manScrollNext) {
    autoScroll = false;
    manScrollNext = true;
    ts.enable(MANUAL_FASTSCROLL_TASK);
    //ts.enable(RETURN_TO_AUTOSCROLL_TASK);
    ts.remove(DEEP_SLEEP_TASK);//, 75000, [&](void *) { goToDeepSleep();}, nullptr, false);
    //ts.disable(DEEP_SLEEP_TASK);
    // Serial.println("! OH! You touched me on the GPIO 27! <touch_callback>");
  }

}

void printTouchVal() {
  Serial.printf("\n==| Touch: %d |==\n\n", touchRead(TOUCH7_AKA_GPIO27));
}

void initTouchInput() {

  touchAttachInterrupt(TOUCH7_AKA_GPIO27, touch_callback, TOUCH_THRESHOLD);

}

#endif