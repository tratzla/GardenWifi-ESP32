#ifndef GW_SLEEP
#define GW_SLEEP

#include <Arduino.h>
#include "heltec.h"
// #include "GW-touch.h"

#define TIME_TO_WAKE_MS 45000
#define TIME_TO_SLEEP_S 30
#define SEC_TO_MICROS_SEC 1000000

long naptime;
initSleep() {
  naptime = millis() + TIME_TO_WAKE_MS;
}



void goToDeepSleep() {
  if(naptime < millis()) return;

  //Wifi.end();

  delay(100);
  esp_sleep_enable_touchpad_wakeup();
  Serial.println("Enabled touch interrupt on TOUCH_PAD_NUM7 aka GPIO 27");

  delay(100);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * SEC_TO_MICROS_SEC);
  Serial.printf("\n\nSleep timer set at %d seconds\n", TIME_TO_SLEEP);


  delay(100);
  Serial.println("Going to DEEP sleep now!");
  delay(2);
  esp_deep_sleep_start();
  Serial.println("!!!This will never be printed");
}



void print_wakeup_touchpad(){
  touch_pad_t pin;

  pin = esp_sleep_get_touchpad_wakeup_status();

  switch(pin)
  {
    case 0  : Serial.println("!Touch detected on GPIO 4"); break;
    case 1  : Serial.println("!Touch detected on GPIO 0"); break;
    case 2  : Serial.println("!Touch detected on GPIO 2"); break;
    case 3  : Serial.println("!Touch detected on GPIO 15"); break;
    case 4  : Serial.println("!Touch detected on GPIO 13"); break;
    case 5  : Serial.println("!Touch detected on GPIO 12"); break;
    case 6  : Serial.println("!Touch detected on GPIO 14"); break;
    case 7  : Serial.println("!Touch detected on GPIO 27"); break;
    case 8  : Serial.println("!Touch detected on GPIO 33"); break;
    case 9  : Serial.println("!Touch detected on GPIO 32"); break;
    default : Serial.println("!Wakeup not by touchpad"); break;
  }
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

   switch(wakeup_reason)  {
    case ESP_SLEEP_WAKEUP_EXT0 : 
        Serial.println("Wakeup caused by external signal using RTC_IO"); 
        break;
    case ESP_SLEEP_WAKEUP_EXT1 : 
        Serial.println("Wakeup caused by external signal using RTC_CNTL"); 
        break;
    case ESP_SLEEP_WAKEUP_TIMER : 
        Serial.println("Wakeup caused by timer"); 
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
        Serial.print("!\n!Wakeup caused by touchpad.   Hello There!\n!..."); 
        print_wakeup_touchpad();
        break;
    case ESP_SLEEP_WAKEUP_ULP : 
        Serial.println("Wakeup caused by ULP program"); 
        break;
    default : 
        Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
        break;
  }
}
#endif