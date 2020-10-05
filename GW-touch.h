#ifndef GW_TOUCH
#define GW_TOUCH


#define TOUCH6_AKA_GPIO14 14
#define TOUCH7_AKA_GPIO27 27
#define TOUCH_THRESHOLD 25 /* 40 for Arduino(150 for ESP) Larger is more sensetive */





void printTouchVal() {
  Serial.printf("\n==| TOUCH7_AKA_GPIO27: %d |==\n\n", touchRead(TOUCH7_AKA_GPIO27));
  Serial.printf("\n==| TOUCH6_AKA_GPIO14: %d |==\n\n", touchRead(TOUCH6_AKA_GPIO14));
}



#endif