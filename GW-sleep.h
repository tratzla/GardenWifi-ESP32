#ifndef GW_SLEEP
#define GW_SLEEP

#include <Arduino.h>
#include "GW-influx.h"
#include "battery.h"

// #define TIME_TO_WAKE_MS 30000
// #define TIME_TO_SLEEP_S 10
#define MIN_TO_MICROS_SEC 60000000
#define SEC_TO_MILLI_SEC 1000

#define SHTDN_NOSHUTDOWN 0
#define SHTDN_INIT_SLEEP 1
#define SHTDN_SLEEP_NOW  2
#define SHTDN_LOW_BATT   4

ulong naptime;
ulong now;
byte shutdownFlags = SHTDN_NOSHUTDOWN;
uint TIME_SLEEPING = 1;  // minutes
uint TIME_AWAKE    = 300; // seconds

void initSleep() {
  naptime = millis() + TIME_AWAKE * SEC_TO_MILLI_SEC;
}


void goToDeepSleep() {
  
  if (lowBattFlag) {
    log_e("[E] - LOW BATTERY detectec. Shutting DOWN!");
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);

  } else {
    delay(100);
    esp_sleep_enable_touchpad_wakeup();
    log_i("Enabled touch WAKEUP on TOUCH_PAD_NUM7(aka GPIO27)");

    delay(100);
    gpio_hold_en(GPIO_NUM_12);
    gpio_hold_en(GPIO_NUM_13);
    gpio_deep_sleep_hold_en();

    delay(100);
    esp_sleep_enable_timer_wakeup(TIME_SLEEPING * MIN_TO_MICROS_SEC);
    log_w("\n\nSleep timer set at %d minutes\n", TIME_SLEEPING);
    
  }

  delay(100);
  log_w("Going to DEEP sleep now!");
  delay(2);
  esp_deep_sleep_start();
}



void print_wakeup_touchpad(){
  touch_pad_t pin;

  pin = esp_sleep_get_touchpad_wakeup_status();

  switch(pin)
  {
    case 0  : log_i("!Touch detected on GPIO 4"); break;
    case 1  : log_i("!Touch detected on GPIO 0"); break;
    case 2  : log_i("!Touch detected on GPIO 2"); break;
    case 3  : log_i("!Touch detected on GPIO 15"); break;
    case 4  : log_i("!Touch detected on GPIO 13"); break;
    case 5  : log_i("!Touch detected on GPIO 12"); break;
    case 6  : log_i("!Touch detected on GPIO 14"); break;
    case 7  : log_i("!Touch detected on GPIO 27"); break;
    case 8  : log_i("!Touch detected on GPIO 33"); break;
    case 9  : log_i("!Touch detected on GPIO 32"); break;
    default : log_i("!Wakeup not by touchpad"); break;
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
        log_i("Wakeup caused by external signal using RTC_IO"); 
        break;
    case ESP_SLEEP_WAKEUP_EXT1 : 
        log_i("Wakeup caused by external signal using RTC_CNTL"); 
        break;
    case ESP_SLEEP_WAKEUP_TIMER : 
        log_i("Wakeup caused by timer"); 
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
        log_i("!\n!Wakeup caused by touchpad.   Hello There!\n!..."); 
        print_wakeup_touchpad();
        break;
    case ESP_SLEEP_WAKEUP_ULP : 
        log_i("Wakeup caused by ULP program"); 
        break;
    default : 
        log_i("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
        break;
  }
}
#endif