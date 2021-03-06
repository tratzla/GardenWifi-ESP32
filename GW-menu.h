#ifndef GW_MENU
#define GW_MENU

#include "GW-header.h"
#include "heltec.h"
#include "font_dialog.h"
#include "GW-sensors.h"
#include "GW-display.h"
#include "GW-datalog.h"
#include "GW-sleep.h"
#include "GW-nvs.h"

// #include "icons/icons.h"
#define SCREEN_SET_NORMAL 1
#define SCREEN_SET_MENU 2

byte screenSet = SCREEN_SET_NORMAL;

#define font_title Dialog_plain_14
#define font_items_s ArialMT_Plain_10
#define font_items Dialog_plain_12

#define ButtonPressed_Both       (uiControl&UI_CTL_BOTH)==(UI_CTL_BOTH)
#define ButtonPressed_Select   (uiControl&UI_CTL_SELECT)==(UI_CTL_SELECT)
#define ButtonPressed_Change   (uiControl&UI_CTL_CHANGE)==(UI_CTL_CHANGE)

#define ButtonUnpress_Both     uiControl&=~UI_CTL_BOTH
#define ButtonUnpress_Select   uiControl&=~UI_CTL_SELECT
#define ButtonUnpress_Change   uiControl&=~UI_CTL_CHANGE

#define _LINE_H 12
#define _LINE_TITLE  0
#define _LINE_ONE   14
#define _LINE_TWO   26
#define _LINE_THREE 38
#define _LINE_FOUR  50

#define _LEFT 0
#define _MIDDLE screen_width/2
#define _RIGHT screen_width


#define MENU_TT100 1
#define MENU_TT101 2
#define MENU_MT200 3
#define MENU_MT201 4
#define MENU_SLEEP 5

#define MENU_FINAL_SCREEN 5
#define MENU_FIRST_SCREEN 1



void menuDrawConfigTitle(String title){
  Heltec.display->setFont(Dialog_plain_14);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(_LEFT + 5, _LINE_TITLE, title);


  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(_RIGHT, _LINE_TITLE, "[CONFIG]");

  Heltec.display->drawLine(0+15,_LINE_H+2, _RIGHT-15, _LINE_H+2);
  
}

void menuDrawLine(uint line_y, String left, String right, uint selected_line, bool ctl_selection){
  String toPrint = left;

  if (selected_line == line_y) {    
    if(ctl_selection) {
      Heltec.display->drawRect(_MIDDLE+5, line_y+1, _MIDDLE-5, _LINE_H+1);
    } else {    
      toPrint = "  " + left;
      Heltec.display->fillCircle(_LEFT+2, line_y+_LINE_H/2, 2);
    }
  }
  
  Heltec.display->setFont(Dialog_plain_12);  
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(_RIGHT-2, line_y, right);

  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(_LEFT, line_y, toPrint);  
}



void changeSleepConfigVal(uint ctl_selection){
  
  if (ctl_selection == _LINE_ONE) {
    TIME_SLEEPING += TIME_SLEEPING > 60 ? -59 : 2;
  }
  if (ctl_selection == _LINE_TWO) {
    TIME_AWAKE += TIME_AWAKE > 60 ? -45 : 5;
  }
}


void showConfigSleep(uint &selected_line, uint ctl_selection){
  String toPrint;
  menuDrawConfigTitle("Sys Sleep");
  
  if(selected_line > _LINE_TWO)
    selected_line = _LINE_ONE;

  toPrint = String(TIME_SLEEPING) + "min";
  menuDrawLine(_LINE_ONE, "Sleeping", toPrint, selected_line, ctl_selection);

  toPrint = String(TIME_AWAKE) + "sec";
  menuDrawLine(_LINE_TWO, "Awake", toPrint, selected_line, ctl_selection);
}


void showConfigSensor(String name, alm_cfg_t *alarm, uint &selected_line, uint ctl_selection){
  String toPrint;
  menuDrawConfigTitle(name);

  if (alarm->in_use) {
    toPrint = "Enabled";
    menuDrawLine(_LINE_ONE, "Alarming", toPrint, selected_line, ctl_selection);

    toPrint = String(alarm->sp, 1);
    menuDrawLine(_LINE_TWO, "Setpoint", toPrint, selected_line, ctl_selection);


    switch(alarm->compare) {
      case COMP_GT: toPrint = String("> [gt]"); break;
      case COMP_LT: toPrint = String("< [lt]"); break;
      case COMP_EQ: toPrint = String("= [eq]"); break;
    }
    menuDrawLine(_LINE_THREE, "Compare", toPrint, selected_line, ctl_selection);

    switch(alarm->state) {
      case NORMAL : toPrint = String("OK")   ; break;
      case ALARM  : toPrint = String("ALARM"); break;
      case UNKNOWN: toPrint = String("??")   ; break;
    }
    menuDrawLine(_LINE_FOUR, "Status", toPrint, selected_line, ctl_selection);

  } else {
    if (selected_line >= _LINE_TWO)
      selected_line = _LINE_ONE;

    toPrint = "Disabled";
    menuDrawLine(_LINE_ONE, "Alarming", toPrint, selected_line, ctl_selection);

  }
}




void refreshMenuDisplay() {
  static uint menuSequence = MENU_TT100;
  static uint selected_line = _LINE_ONE;
  static uint ctl_selection = 0;

  /* 
   * Reading Buttons for user input
  */
  if(ButtonPressed_Both) {
    if(ctl_selection == 0) {
      menuSequence++;
      selected_line = _LINE_ONE;
    }
    ButtonUnpress_Both;
  }

  if(ButtonPressed_Select) {
    if(ctl_selection > 0) {
      switch(menuSequence)
      {
        case MENU_TT100: TT100.saveAlmConfig_nvs(); break;
        case MENU_TT101: TT101.saveAlmConfig_nvs(); break;
        case MENU_MT200: MT200.saveAlmConfig_nvs(); break;
        case MENU_MT201: MT201.saveAlmConfig_nvs(); break;
        case MENU_SLEEP: saveSleeps_nvs(TIME_AWAKE, TIME_SLEEPING); break;
      }
      ctl_selection = 0;
      alarm_config_unsaved_change = true;
    } else {
      if(selected_line >= _LINE_FOUR)
        selected_line = _LINE_ONE;
      else
        selected_line += _LINE_H;
    }
    ButtonUnpress_Select;
  }

  if(ButtonPressed_Change) {
    if(ctl_selection == 0) {
      ctl_selection = selected_line;
    } else {
      uint whichParam;
      switch(ctl_selection)
      {
        case _LINE_ONE  : whichParam = CFG_CHNG_INUSE; break;
        case _LINE_TWO  : whichParam = CFG_CHNG_SP   ; break;
        case _LINE_THREE: whichParam = CFG_CHNG_COMP ; break;
      }

      switch(menuSequence)
      {
        case MENU_TT100: TT100.changeConfig(whichParam); break;
        case MENU_TT101: TT101.changeConfig(whichParam); break;
        case MENU_MT200: MT200.changeConfig(whichParam); break;
        case MENU_MT201: MT201.changeConfig(whichParam); break;
        case MENU_SLEEP: 
          if(ctl_selection == _LINE_ONE)
            TIME_SLEEPING += TIME_SLEEPING > 90 ? -91 : 3;
          if(ctl_selection == _LINE_TWO)
            TIME_AWAKE    += TIME_AWAKE > 300 ? -300 : 10;
          break;
      }
    }
    ButtonUnpress_Change;
  }


  /* 
   * Start Drawing the Appropriate Screen
  */
  if(menuSequence > MENU_FINAL_SCREEN) {
    menuSequence = MENU_FIRST_SCREEN;  
    screenSet = SCREEN_SET_NORMAL;
  }

  Heltec.display->clear();
    if (menuSequence == MENU_TT100) {
      TT100.strAlarm_Compare();
      showConfigSensor(TT100.name, TT100.AlarmConfig, selected_line, ctl_selection);
    }
    if (menuSequence == MENU_TT101) {
      TT101.strAlarm_Compare();
      showConfigSensor(TT101.name, TT101.AlarmConfig, selected_line, ctl_selection);
    }
    if (menuSequence == MENU_MT200) {
      MT200.strAlarm_Compare();
      showConfigSensor(MT200.name, MT200.AlarmConfig, selected_line, ctl_selection);
    }
    if (menuSequence == MENU_MT201) {
      MT201.strAlarm_Compare();
      showConfigSensor(MT201.name, MT201.AlarmConfig, selected_line, ctl_selection);
    }
    if (menuSequence == MENU_SLEEP) {
      showConfigSleep(selected_line, ctl_selection);
    }

  Heltec.display->display();
}


#endif