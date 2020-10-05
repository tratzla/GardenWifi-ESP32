#ifndef GW_MENU
#define GW_MENU

#include "GW-header.h"
#include "heltec.h"
#include "font_dialog.h"
#include "GW-sensors.h"
#include "GW-display.h"
#include "GW-datalog.h"

// #include "icons/icons.h"
// #include "GW-sleep.h"


#define font_title Dialog_plain_14
#define font_items_s ArialMT_Plain_10
#define font_items Dialog_plain_12



#define _LINE_H 12
#define _LINE_TITLE  0
#define _LINE_ONE   16
#define _LINE_TWO   28
#define _LINE_THREE 40
#define _LINE_FOUR  52

#define _LEFT 0
#define _MIDDLE screen_width/2
#define _RIGHT screen_width

#define MENU_TT1100 1
#define MENU_TT1101 2
#define MENU_FINAL_SCREEN 2


#define MENU_CTL_SELECT 1
#define MENU_CTL_CHANGE 2

byte menuControl = 0;



void menuDrawConfigTitle(String title){
  Heltec.display->setFont(Dialog_plain_14);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(_MIDDLE-16, _LINE_TITLE, title);


  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(_RIGHT, _LINE_TITLE, "[CONFIG]");

  // Heltec.display->setFont(ArialMT_Plain_10);
  // Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  // Heltec.display->drawString(_LEFT, _LINE_TITLE, "cfg");
}

void menuDrawLine(uint line_y, String left, String right, uint selected_line){
  String toPrint = left;
  if (selected_line == line_y) {
    toPrint = "  " + left;
    Heltec.display->fillCircle(_LEFT+2, line_y+_LINE_H/2, 2);
  }
  Heltec.display->setFont(Dialog_plain_12);  
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(_RIGHT, line_y, right);

  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(_LEFT, line_y, toPrint);  
}

void showConfigDht(DhtSensor &sensor, uint selected_line){
  String toPrint;
  menuDrawConfigTitle(sensor.name);
  
  toPrint = sensor.alarm.in_use ? "Enabled" : "Disabled";
  menuDrawLine(_LINE_ONE, "Alarming", toPrint, selected_line);
  
  if (sensor.alarm.in_use) {
    toPrint = String(sensor.alarm.sp, 1);
    menuDrawLine(_LINE_TWO, "Setpoint", toPrint, selected_line);

    toPrint = String(sensor.alarm.charCompare());
    menuDrawLine(_LINE_THREE, "Compare", toPrint, selected_line);

    toPrint = String(sensor.alarm.strState());
    menuDrawLine(_LINE_FOUR, "Status", toPrint, selected_line);

  }
  
}




void refreshMenuDisplay() {
  static uint menuSequence = 1;
  static uint selected_line = _LINE_ONE;
  static bool ctl_selection = false;
  static bool ctl_change = false;


  //if (resetSelection) selected_line = _LINE_ONE;

  Heltec.display->clear();

  if (menuSequence == MENU_TT1100) {
    showConfigDht(TT100, selected_line);
  }
  if (menuSequence == MENU_TT1101) {
    showConfigDht(TT101, selected_line);
  }

  Heltec.display->display();

  if (menuControl == MENU_CTL_SELECT & MENU_CTL_CHANGE ) {
    menuSequence++;
    selected_line = _LINE_ONE;
    menuControl = menuControl & !MENU_CTL_SELECT & !MENU_CTL_CHANGE;
  }
  if (menuSequence > MENU_FINAL_SCREEN) {
    menuSequence = MENU_TT1100;  
  }

  if (menuControl & MENU_CTL_CHANGE) {
    Serial.printf("Change a thing in menu\n");
    menuControl = menuControl & !MENU_CTL_CHANGE;
  }

  if (menuControl & MENU_CTL_SELECT) {
    Serial.printf("New line in menu (%d)(%d)(%d)\n",menuControl , MENU_CTL_SELECT, menuControl & MENU_CTL_SELECT==0);
    menuControl = menuControl & !MENU_CTL_SELECT;
    selected_line += _LINE_H; 
  }
  if (selected_line > _LINE_FOUR){
    selected_line = _LINE_ONE;
  }

//   if (!ctl_selection && menuControl | MENU_CTL_SELECT){
//     ctl_selection = true;
//   }
//   if (ctl_selection && (menuControl | MENU_CTL_SELECT == 0))
//     ctl_selection = (ctl_selection && menuControl | MENU_CTL_SELECT);
}


#endif