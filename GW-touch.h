#ifndef GW_TOUCH
#define GW_TOUCH


#define TOUCH7_AKA_GPIO27 27
#define TOUCH_THRESHOLD 40 /* 40 for Arduino(150 for ESP) Larger is more sensetive */

bool pin27Touched = true;

void touch_callback() {
  // Serial.println("! OH! You touched me on the GPIO 27! <touch_callback>");
  //   manScrollNext = true; 
  pin27Touched = true;
}



void printTouchVal() {
  Serial.printf("\n==| Touch: %d |==\n\n", touchRead(TOUCH7_AKA_GPIO27));
}

void initTouchInput() {
  touchAttachInterrupt(TOUCH7_AKA_GPIO27, touch_callback, TOUCH_THRESHOLD);
}

#endif