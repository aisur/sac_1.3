#include <SHT1x.h>
#include <SoftwareSerial.h>
#include <SerLCD.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DS1307RTC.h>
#include <Wire.h>

#include <Time.h>
#include <time.h>

/****************************************************************
 * SAC: This is the Main File of SAC Project:
 * http://saccultivo.com
 * 
 * IN this file we can see all the functions for the correct functionality of the SAC Unit for 3 Output Channel
 * 
 * Author: Victor Suarez Garcia<suarez.garcia.victor@gmail.com>
 * Co-Author: David Cuevas Lopez<mr.cavern@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * Version History:
 * 1.1. Initial Version. Written by Øyvind Kolås pippin@gimp.org in 2013.
 * 1.2. Some Changes for more readablity. written by victor suarez suarez.garcia.victor@gmail.com and David Cuevas mr.cavern@gmail.com in March 2014.
 * 1.3. Improved all the Functionality for 20x4 Screen and improved for 1.3 PCB Version. written by victor suarez suarez.garcia.victor@gmail.com and David Cuevas mr.cavern@gmail.com in April 2014
 * 
 * 
 * Current Version: 1.3.
 *************************************************************************/


#include "LCDUtils.h"
#include "RTCUtils.h"
#include "EEPROMUtils.h"
#include "languages.h"
#include "SACSensors.h"
#include "Relay.h"
/*
*LCD CONFIG & PINS
 */
#define LCD_PIN 11
#define NUM_COLS 20
#define NUM_ROWS 4
#define MAXMENUITEMS 6
#define magia(lcd,x) {  lcd.setPosition(1,0); lcd.print(x);delay(5000);}
#define printTitle(lcd,m){lcd.print("***");lcd.print(m);lcd.print("***");}
/*
 * BUTTONS PINS
 */
#define BUTTON_UP_PIN 9
#define BUTTON_CENTER_PIN 8
#define BUTTON_DOWN_PIN 7

#define BUTTONUP 0
#define BUTTONDOWN 1
#define BUTTONCENTER 2
#define BUTTONCENTERLONG 3
#define BUTTONUPLONG 4
#define BUTTONDOWLONG 5
#define TIMEOUT 6

#define IDLE -1

#define VERSION 1.3


/*
 * DIFFERENT STATES TO MOVE THROUGH MENU
 */
enum States
{
  INICIO,
  SELECTIDIOMA,
  CALIB_SAT,
  ESTADO,
  EDICION,
  SELECCION,
  ROLEMENU,
  ROLESELECT
};
/*
 * DIFFERENT STATES IN SELECTION MODE
 */
enum Seleccion_States
{
  MENU,
  FECHA,
  HORA,
  CALIBRACION_SAT,
  RESET_CONFIG,
  ABOUT,
  END_SELECTION
};
/*
 * DIFFERENT STATES IN SELECTION MODE FOR DATE MENU
 */
enum selectionDateStates
{
  DAY,
  MONTH,
  YEAR,
  SAVE,
  BACK
};
/*
 * DIFFERENT STATES IN SELECTION MODE FOR TIME MENU
 */
enum s_selectionTimeStates
{
  S_SELECTHOURS,
  S_SELECTMINUTES,
  S_SAVETIME,
  S_BACKTIME 
};
enum s_selectStatus
{
  S_HSO=0,
  S_HSMIN,
  S_PCICLE,
  S_PINTERVAL,
  S_TSMAX,
  S_TSMIN,

};
/*
 * MENU STRUCTURE 
 */
typedef struct MenuItem
{
  int label;
  int  state; 
};
MenuItem main_menu[] ={
  {
    S_DATE,FECHA        }
  ,
  {
    S_HOUR,HORA        }
  ,{
    S_SATCALIBRATION,CALIBRACION_SAT        }
  ,
  {
    S_RESET,RESET_CONFIG        }
  ,{
    S_ABOUT,ABOUT    }
  ,{
    S_RETURN_TO,END_SELECTION        }

};

int current_menu;

int select_language;
/*
 * LCD PIN SETUP & CONFIG
 */
SoftwareSerial SSerial(0,LCD_PIN);
SerLCD mylcd(SSerial,NUM_COLS,NUM_ROWS);
/*
 * VALUES, STATES & CONFIG
 */
boolean actualizar_pantalla;
Configuration current_config;
tmElements_t tm;
cached_sensors current_sensorsvalues;
State current_state;
State previous_state;

/*
 * RELAY CONFIG
 *EACH RELAY USES A ROLE TO CONTROL IT'S FUNCTIONS
 */


Relay relay[MAX_RELAYS]={
  {
    RELAY1_PIN,R_IRRIGATION,RELAY_OFF     }
  ,{
    RELAY2_PIN,R_HUMIDIFIER,RELAY_OFF          }
  ,{
    RELAY3_PIN,R_COOLING,RELAY_OFF     }
};
/*
 * GLOBAL VARIABLES
 */
tmElements_t lastUpdate;
boolean irrigating;
byte current_mstate;
byte current_selectionstate;
byte current_selectionDateState;
byte cTime=S_SELECTHOURS;
byte button_up_state=LOW;
byte button_down_state=LOW;
byte button_center_state=LOW;
byte center_pressed_state=0;
byte up_pressed_state=0;
byte selectionStatus=S_HSO;
byte editHours;
byte editMinutes;
byte editDays;
byte editMonths;
byte current_rele=0;
int editYears;
boolean isEditing;
long time1;
long time2;
//--------------------------------------------------------------------------
//SACLCD saclcd(mylcd);

void setup()
{
  Serial.begin(9600);
  SSerial.begin(9600);
  mylcd.begin();
  setup_pins();
  actualizar_pantalla=true;
  // setupFlowRate();

  irrigating=false;
  current_selectionstate=MENU;
  if(!load_Settings(current_config)){
    current_config=reset_Settings();
    current_mstate=SELECCION;
  }
  else{
    current_mstate=ROLEMENU; 
  }
  initializeGlobalVars();
}
/*Initialize Global Variables. */
void initializeGlobalVars(){
  active_language =current_config.active_languaje;
  current_sensorsvalues.cached_tempmax=(current_config.temps_max!=0)?current_config.temps_max:35;
  current_sensorsvalues.cached_tempmin=(current_config.temps_min!=0)?current_config.temps_min:8;
  current_sensorsvalues.cached_minmoisture=(current_config.moisture_min!=0)?current_config.moisture_min:30;
  current_sensorsvalues.cached_maxmoisture=(current_config.moisture_target!=0)?current_config.moisture_target:70;
  current_sensorsvalues.cached_cicle_length=(current_config.pump_cicle_length!=0)?current_config.pump_cicle_length:15;
  current_sensorsvalues.cached_pump_percent=(current_config.pump_percent!=0)?current_config.pump_percent:100;
  current_sensorsvalues.cached_airTAO=(current_config.airTAO!=0)?current_config.airTAO:35;
  current_sensorsvalues.cached_airHMIN=(current_config.airHMIN!=0)?current_config.airHMIN:25;
  current_sensorsvalues.cached_airHRO=(current_config.airHRO!=0)?current_config.airHRO:75;
  current_sensorsvalues.cached_airTMIN=(current_config.airTMIN!=0)?current_config.airTMIN:5;
  time1=millis();
}
/**
 * setup SAC Pins.
 */
void setup_pins(){
  pinMode(BUTTON_UP_PIN,INPUT);
  pinMode(BUTTON_DOWN_PIN,INPUT);
  pinMode(BUTTON_CENTER_PIN,INPUT);

  pinMode(SOIL_MOISTURE_POWER_PIN, OUTPUT);
  for(int i=0;i<MAX_RELAYS;i++)
  {
    pinMode(relay[i].gpio_pin,OUTPUT);

  }

  isEditing=false;
}
void loop(){


  RTCread(tm);

  //ALWAYS UPDATE SCREEN WHEN STATE CHANGES
  update_State(current_sensorsvalues,tm,current_config.calib_FCapacity);
  current_state=read_sensors(current_sensorsvalues);
  if(current_mstate==ESTADO|| current_mstate==ROLEMENU){
  if(state_changed(current_state,previous_state) || time_between(lastUpdate,tm)>1){
    actualizar_pantalla=true; 
    previous_state=current_state;
    lastUpdate=tm;
  }
  else{
    actualizar_pantalla=false; 
  }
  }

  int event=get_event();
 // magia(mylcd,freeRam());


  handleEvent(event);
  update_relay_state();
  drawUI(current_state);
  delay(250);

}

/*Gets the current Button Event
 * returns: the button that is pressed.
 *  0: button UP is Pressed.
 *  1: button DOWN is Pressed.
 *  2: button CENTER is Pressed minor than 3 seconds.
 *  3: button CENTER is Pressed more than 3 seconds.
 */
int get_event(){
  int event=IDLE;
  event=button_up_pressed();
  if(event!=IDLE){ return event;}
  
  if(button_down_pressed()==HIGH) return BUTTONDOWN;
  time2=millis();
   event=button_center_pressed();
  if(event>=0){
     time1=millis(); 
  }
  if((time2-time1)>=30000){
    time1=millis();
    event= TIMEOUT;
  }
  Serial.println(event);
  return event;

}
/*
* Check if button up is pressed. The Event only ocurrs when the button is released; not
 * when button is pressed.
 returns HIGH when button Up is released or LOW otherwise.
 */
int button_up_pressed()
{
  byte current_state=digitalRead(BUTTON_UP_PIN);

  if(current_state==HIGH){
    if(up_pressed_state>=7 ){
      up_pressed_state=0;
    return BUTTONUPLONG;
    }else{
        up_pressed_state++;
    }
   
  }
  else{
   if(up_pressed_state<7 && up_pressed_state>0)
   {
   up_pressed_state=0;
    return BUTTONUP;
   }
  }
  return IDLE;

}
/*
* check if button down is pressed. The Event only ocurrs when the Button is released. NOt when
 * the button is Pressed.
 * returns HIGH when the Button is released or LOW otherwise.
 */
int button_down_pressed()
{
  byte current_state=digitalRead(BUTTON_DOWN_PIN);


  if(button_down_state==HIGH && current_state==LOW){
    button_down_state=current_state;
    return HIGH;
  }
  else{
    button_down_state=current_state;
    return LOW;
  }
}
/*
* check if button center is pressed with two states. When is pressed minor than 3 secods or if its pressed
 more than 3 seconds. The event only ocurr when the button is released. not when is pressed.
 returns: -1 if is not released, 2 if button is pressed minor than three seconds or 3 if  button is pressed
 more than 3 seconds.
 */
int button_center_pressed()
{
  byte current_state=digitalRead(BUTTON_CENTER_PIN);

  if(current_state==HIGH){
    if(center_pressed_state>=7 ){
      center_pressed_state=0;
    return BUTTONCENTERLONG;
    }else{
        center_pressed_state++;
    }
   
  }
  else{
   if(center_pressed_state<7 && center_pressed_state>0)
   {
   center_pressed_state=0;
    return BUTTONCENTER;
   }
  }
  return IDLE;

}

/*
 * HANDLE EVENTS FOR STATE SCREEN & SELECT MODE
 * CASE ESTADO: STATUS SCREEN, DEFAULT VIEW.
 * CASE SELECCION: SELECTION STATE, CALLS THE EVENT HANDLER FOR THIS STATE
 */
void handleEvent(int event)
{
   if(event==BUTTONCENTERLONG){
      //Clear LCD Screen
      mylcd.clear();
      current_mstate=SELECCION;
      current_menu=0; 
      actualizar_pantalla=true;
      return;
    }
  switch(current_mstate)
  {
  case ESTADO:
   
    if(event==BUTTONCENTER)
    {
      mylcd.clear();
      current_mstate=EDICION;
      actualizar_pantalla=true;
      selectionStatus=S_HSO;
    }
    if(event==TIMEOUT)
    {
    mylcd.clear();
    current_mstate=ROLEMENU;
    mylcd.boxCursorOff();
    actualizar_pantalla=true;
    }
    break; 
  case SELECCION:

    handleEventSelection(event);

    break;
  case EDICION:

    handleEventRoleEditionStatus(event);
    
    break;
   case ROLEMENU:
     if(event==BUTTONCENTER)
       {
         current_mstate=ROLESELECT;
         actualizar_pantalla=true;
       }
   break;
   case ROLESELECT:
   handleEventRoleSelectStatus(event);
   break;
  }
}
void handleEventRoleSelectStatus(int event)
{
  if(event==BUTTONDOWN)
  {
    current_rele++;
    current_rele%=3;
    actualizar_pantalla=true;
  }
  if(event==BUTTONUP)
  {
    current_rele--;
    current_rele=(current_rele<0)? 2:current_rele;
    actualizar_pantalla=true;
  }
  if(event==BUTTONCENTER)
  {
    current_mstate=ESTADO;
    actualizar_pantalla=true;
    mylcd.boxCursorOff();
    mylcd.clear(); 
  }
  if(event==TIMEOUT)
  {
    current_mstate=ROLEMENU;
    mylcd.boxCursorOff();
    actualizar_pantalla=true;
  }
}
void handleEventRoleEditionStatus(int event)
{
   switch(relay[current_rele].role)
  {
   case R_IRRIGATION:
      handleEventSelectStatus(event);
   break;
   case R_COOLING:
      handleEventSelectCoolingStatus(event);
   break;
   case R_HUMIDIFIER:
       handleEventSelectHumidifierStatus(event);
   break;
   
  } 
}
void handleEventSelectStatus(int event)
{
  if(event==BUTTONCENTER)
  {
    isEditing=!isEditing;
    actualizar_pantalla=true; 
  }
  if(!isEditing){
    if(event==BUTTONDOWN)
    {
      selectionStatus++;
      selectionStatus%=6;
      actualizar_pantalla=true;
    } 
    if(event==BUTTONUP)
    {
      selectionStatus--;
      selectionStatus=(selectionStatus<0)?5:selectionStatus%6;
      actualizar_pantalla=true;
    }
     
    if(event==TIMEOUT)
    {

      selectionStatus=S_HSO;
      current_mstate=ESTADO;
      actualizar_pantalla=true;
      mylcd.clear();
      mylcd.boxCursorOff(); 
      //Store current state settings
      current_config.moisture_target=current_sensorsvalues.cached_maxmoisture;
      current_config.moisture_min=current_sensorsvalues.cached_minmoisture;
      current_config.temps_max=current_sensorsvalues.cached_tempmax;
      current_config.temps_min=current_sensorsvalues.cached_tempmin;
      current_config.pump_cicle_length=current_sensorsvalues.cached_cicle_length;
      current_config.pump_percent=current_sensorsvalues.cached_pump_percent;
      store_Settings(current_config);
    }

  }
  else{

    switch(selectionStatus)
    {
    case S_HSO:
      if(event==BUTTONDOWN){
        current_sensorsvalues.cached_maxmoisture++;
        current_sensorsvalues.cached_maxmoisture=(current_sensorsvalues.cached_maxmoisture<=current_sensorsvalues.cached_minmoisture)?current_sensorsvalues.cached_minmoisture:(int)current_sensorsvalues.cached_maxmoisture%100;
        actualizar_pantalla=true;
      }
      if(event==BUTTONUP){
        current_sensorsvalues.cached_maxmoisture--;
        current_sensorsvalues.cached_maxmoisture=(current_sensorsvalues.cached_maxmoisture<=current_sensorsvalues.cached_maxmoisture)?current_sensorsvalues.cached_minmoisture:(int)current_sensorsvalues.cached_maxmoisture%100;
        actualizar_pantalla=true;
      }
      if(event==BUTTONUPLONG){
        current_sensorsvalues.cached_maxmoisture-=10;
        current_sensorsvalues.cached_maxmoisture=(current_sensorsvalues.cached_maxmoisture<=current_sensorsvalues.cached_maxmoisture)?current_sensorsvalues.cached_minmoisture:(int)current_sensorsvalues.cached_maxmoisture%100;
        actualizar_pantalla=true;
      }
      break;
    case S_HSMIN:
      if(event==BUTTONDOWN){
        current_sensorsvalues.cached_minmoisture++;
        current_sensorsvalues.cached_minmoisture=(current_sensorsvalues.cached_maxmoisture<=current_sensorsvalues.cached_minmoisture)?current_sensorsvalues.cached_maxmoisture:(int)current_sensorsvalues.cached_minmoisture%100;
        actualizar_pantalla=true;
      }
      if(event==BUTTONUP){
        current_sensorsvalues.cached_minmoisture--;
        current_sensorsvalues.cached_minmoisture=(current_sensorsvalues.cached_maxmoisture<=current_sensorsvalues.cached_minmoisture)?current_sensorsvalues.cached_maxmoisture:(int)current_sensorsvalues.cached_minmoisture%100;
        actualizar_pantalla=true;
      }
      break;
    case S_PCICLE:
        if(event==BUTTONDOWN)
        {
          current_sensorsvalues.cached_cicle_length++;
          current_sensorsvalues.cached_cicle_length=(current_sensorsvalues.cached_cicle_length>=300)?0:current_sensorsvalues.cached_cicle_length;
          actualizar_pantalla=true;
        }
        if(event==BUTTONUP)
        {
          current_sensorsvalues.cached_cicle_length--;
          current_sensorsvalues.cached_cicle_length=(current_sensorsvalues.cached_cicle_length<0)?300:current_sensorsvalues.cached_cicle_length;
          actualizar_pantalla=true;
        }
      break;
    case S_PINTERVAL:
      if(event==BUTTONDOWN)
      {
         current_sensorsvalues.cached_pump_percent++;
         current_sensorsvalues.cached_pump_percent=(current_sensorsvalues.cached_pump_percent>100)?0:current_sensorsvalues.cached_pump_percent;
         actualizar_pantalla=true;
      }
      if(event==BUTTONUP)
      {
         current_sensorsvalues.cached_pump_percent--;
         current_sensorsvalues.cached_pump_percent=(current_sensorsvalues.cached_pump_percent>100)?100:current_sensorsvalues.cached_pump_percent;
         actualizar_pantalla=true;
      }
      break;
    case S_TSMAX:
      if(event==BUTTONUP){
        current_sensorsvalues.cached_tempmax--;
        current_sensorsvalues.cached_tempmax=(current_sensorsvalues.cached_tempmax<=current_sensorsvalues.cached_tempmin)?current_sensorsvalues.cached_tempmin:(int)current_sensorsvalues.cached_tempmax%100;
        actualizar_pantalla=true;
      }
      if(event==BUTTONDOWN){
        current_sensorsvalues.cached_tempmax++;
        current_sensorsvalues.cached_tempmax=(current_sensorsvalues.cached_tempmax<=current_sensorsvalues.cached_tempmin)?current_sensorsvalues.cached_tempmin:(int)current_sensorsvalues.cached_tempmax%100;
        actualizar_pantalla=true;
      }
      break;
    case S_TSMIN:
      if(event==BUTTONUP){
        current_sensorsvalues.cached_tempmax--;
        current_sensorsvalues.cached_tempmax=(current_sensorsvalues.cached_tempmax>=current_sensorsvalues.cached_tempmax)?current_sensorsvalues.cached_tempmax:(int)current_sensorsvalues.cached_tempmin%100;
        actualizar_pantalla=true;
      }
      if(event==BUTTONDOWN){
        current_sensorsvalues.cached_tempmin++;
        current_sensorsvalues.cached_tempmin=(current_sensorsvalues.cached_tempmin>=current_sensorsvalues.cached_tempmax)?current_sensorsvalues.cached_tempmax:(int)current_sensorsvalues.cached_tempmin%100;
        actualizar_pantalla=true;
      }
      break;
    default:

      break;
    }
  }

}

/*
 * EVENT HANDLER FOR EACH SELECTION STATE
 * MENU: MOVE & INTERACT WITH MENU
 * IDIOMA: LANGUAGE SELECTION MENU
 * FECHA: DATE CONFIG MENU
 * HORA: TIME CONFIG MENU
 * END SELECTION: EXITS SELECTION STATE
 */
void handleEventSelection(int event)
{
  switch(current_selectionstate)
  {
  case MENU:

    if(event == BUTTONCENTER)
    {
      mylcd.clear();
      current_selectionstate=main_menu[current_menu].state;
      select_language=0;
      current_selectionDateState=0;
      actualizar_pantalla=true;
      editHours=tm.Hour;
      editMinutes=tm.Minute;
      editDays=tm.Day;
      editMonths=tm.Month;
      editYears=tmYearToCalendar(tm.Year);
      isEditing=false;
      // current_selectTimeState=SELECTHOURS;


    }
    if(event==BUTTONDOWN)
    {
      mylcd.clear();
      current_menu++;
      current_menu= current_menu%MAXMENUITEMS;
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP)
    {
      mylcd.clear();
      current_menu=(current_menu==0)? MAXMENUITEMS-1: current_menu-1;
      current_menu= current_menu%MAXMENUITEMS;
      actualizar_pantalla=true;
    }
    break;
  

  case FECHA:
    if(isEditing)
      handleEventEditingDate(event);
    else
      handleEventSelectionDate(event);

    break;
  case HORA:
    if(isEditing) 
      handleEventEditingHour(event);
    else
      handleEventSelectionHour(event);
    break;
  case CALIBRACION_SAT:
    if(event==BUTTONCENTER)
    {
      int calib=readFCapacityValue();
      current_config.calib_FCapacity=calib;
      store_Settings(current_config);
      current_selectionstate=MENU;
      mylcd.clear();
    }
    actualizar_pantalla=true;
    break;
  case RESET_CONFIG:

    current_config=reset_Settings();
    initializeGlobalVars();
    current_mstate=SELECCION;
    current_selectionstate=MENU;
    actualizar_pantalla=true;
    break;
  case ABOUT:
    if(event==BUTTONCENTER)
    {

      current_selectionstate=MENU;
      actualizar_pantalla=true;
      mylcd.clear();
    }
    break;
  case END_SELECTION:
    current_mstate=ESTADO;
    current_selectionstate=MENU;
    actualizar_pantalla=true;
    break;
  }
}
void handleEventSelectionDate(int event)
{
  switch(current_selectionDateState)
  {


  default:

    if(event==BUTTONUP)
    {
      current_selectionDateState--;
      current_selectionDateState%=5;
      actualizar_pantalla=true;
    } 
    if(event==BUTTONDOWN)
    {
      current_selectionDateState++;
      current_selectionDateState%=5;
      actualizar_pantalla=true;
    }
    if(event==BUTTONCENTER)
    {
      isEditing=true;
      actualizar_pantalla=true;
      mylcd.clear(); 
    }
    if(current_selectionDateState==SAVE && event==BUTTONCENTER)
    {
      current_selectionstate=MENU;
      actualizar_pantalla=true;
      current_menu=0;
      mylcd.clear();
      tm.Day=editDays;
      tm.Month=editMonths;
      tm.Year=CalendarYrToTm(editYears);
      setHour(tm);
    }
    if(current_selectionDateState==BACK && event==BUTTONCENTER)
    {
      current_selectionstate=MENU;
      actualizar_pantalla=true;
      current_menu=0;
      mylcd.clear();
    }
    break;
  }


}
void handleEventSelectionHour(int event)
{
  switch(cTime)
  {
  case S_SAVETIME:
    if(event==BUTTONCENTER)
    {
      tm.Hour=editHours;
      tm.Minute=editMinutes;
      setHour(tm);
      current_selectionstate=MENU;
      actualizar_pantalla=true;
      current_menu=0;
      mylcd.clear();
      cTime=S_SELECTHOURS;
      break;
    }
  case S_BACKTIME:
    if(event==BUTTONCENTER){
      current_selectionstate=MENU;
      actualizar_pantalla=true;
      current_menu=0;
      mylcd.clear();
      cTime=S_SELECTHOURS;
      break;
    }

  default:
    if(event==BUTTONDOWN)
    {
      actualizar_pantalla=true;
      cTime++;
      cTime%= 4; 

    }
    if(event==BUTTONUP)
    {
      actualizar_pantalla=true;
      cTime--;
      cTime= (cTime<0)?4:cTime%4; 

    }
    if(event==BUTTONCENTER)
    {
      actualizar_pantalla=true;
      isEditing=true;
      mylcd.clear();
    }
    break;
  } 
}
void handleEventEditingDate(int event)
{
  switch(current_selectionDateState)
  {
  case DAY:
    if(event==BUTTONDOWN){
      editDays++;
      editDays%=getDaysofMonth(editMonths);
      editDays=(getDaysofMonth(editMonths)>31)?editDays=1:editDays;
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editDays--;
      editDays%=getDaysofMonth(editMonths);
      editDays=(getDaysofMonth(editMonths)<1)?editDays=getDaysofMonth(editMonths):editDays;
      actualizar_pantalla=true;
    }
    if(event==BUTTONCENTER){
      isEditing=false;
      actualizar_pantalla=true;
      mylcd.clear();
    }
    break;
  case MONTH:
    if(event==BUTTONDOWN){
      editMonths++;
      editMonths=(editMonths%12)+1;
      editMonths=(editMonths>12)?editMonths=1:editMonths;
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editMonths--;
      editMonths=(editMonths%12)+1;
      editMonths=(editMonths<1)?editMonths=12:editMonths;
      actualizar_pantalla=true;
    }
    if(event==BUTTONCENTER){
      isEditing=false;
      actualizar_pantalla=true;
      mylcd.clear();
    }
    break;
  case YEAR:
    if(event==BUTTONDOWN){
      editYears++;
      editYears=(editYears>2032)?editYears=2000:editYears;  
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editYears--;
      editYears=(editYears<2000)?editYears=2032:editYears;
      actualizar_pantalla=true;
    }
    if(event==BUTTONCENTER){
      isEditing=false;
      actualizar_pantalla=true;
      mylcd.clear();
    }
    break;
  default:

    break; 
  }

}
void handleEventEditingHour(int event)
{
  switch(cTime)
  {
  case S_SELECTHOURS:
    if(event==BUTTONDOWN){
      editHours++;
      editHours%= 24;
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editHours--;
      editHours=(editHours<0)?23:editHours%24;
      actualizar_pantalla=true;
    }
  case S_SELECTMINUTES:
    if(event==BUTTONDOWN){
      editMinutes++;
      editMinutes%= 60;
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editMinutes--;
      editMinutes=(editMinutes<0)?259:editMinutes%60;
      actualizar_pantalla=true;
    }
  default:
    if(event==BUTTONCENTER){
      isEditing=false;
      actualizar_pantalla=true;
      mylcd.clear();
    }
    break; 
  }

}

/*
 * DRAWS  INTERFACE IN STATUS MODE & SELECTION MODE
 */
void drawUI(State & state){
  if(actualizar_pantalla){
    switch(current_mstate)
    {
    case SELECCION:
      drawSeleccion();
      break;
    case  ESTADO:

      drawState(state);
      break;
    case EDICION:
      drawSelectStatus(state);
      break;
    case ROLEMENU:
     drawRoleMenu(state);

     break;
     case ROLESELECT:
     drawSelectRoleMenu(state);
     break;
    }
    actualizar_pantalla=false;
  }

}
/*Draw The Current State Role
 * Parameters:
 * state: The struct with the Current State.
 */
void drawState(State & state)
{
  byte role= relay[current_rele].role;
  switch(role)
  {
    case R_IRRIGATION:
     drawIrrigationState(state);
    break;
    case R_HUMIDIFIER:
    drawHumidificationState(state);
    break;
   case R_COOLING:
    drawCoolingState(state);
   break; 
  }
}
void drawSelectRoleMenu(State & state)
{
  mylcd.setPosition(1,0);
  if(tm.Hour<10) mylcd.print("0");
  mylcd.print(tm.Hour);
  mylcd.print(":");
  if(tm.Minute<10) mylcd.print("0");
  mylcd.print(tm.Minute);
  mylcd.print("  ");
  if(tm.Day<10) mylcd.print("0");
  mylcd.print(tm.Day);
  mylcd.print("/");
  if(tm.Month<10) mylcd.print("0");
  mylcd.print(tm.Month);
  mylcd.print("/");
  mylcd.print(tmYearToCalendar(tm.Year));
  for (int i=0; i<MAX_RELAYS;i++){
   mylcd.setPosition(i+2,0);
   mylcd.print("C");
   mylcd.print(i+1);
   mylcd.print(" ");
   drawRoleInfo(relay[i].role,state);
  }
  switch(current_rele)
 {
   case 0:
     mylcd.setPosition(2,1);
     mylcd.boxCursorOn();
   break;
   case 1:
     mylcd.setPosition(3,1);
     mylcd.boxCursorOn();
   break;
   case 2:
    mylcd.setPosition(4,1);
     mylcd.boxCursorOn();
   break;
 } 
}

void drawRoleInfo(byte role, State & state){
  switch (role){
    case R_IRRIGATION:
    mylcd.print(translate(S_IRRIGATION));
    mylcd.print(" ");
    mylcd.print(translate(S_HS));
    mylcd.print(state.current_moisture);
    mylcd.print("%");
    break;
    case R_COOLING:
    mylcd.print(translate(S_REFRIG));
    mylcd.print(" ");
    mylcd.print(translate(S_TA));
    mylcd.print(state.current_airTemperature);
    mylcd.print("C");
    break;
    case R_HUMIDIFIER:
    mylcd.print(translate(S_HUMIDIF));    
    mylcd.print(" ");
    mylcd.print(translate(S_HR));
    mylcd.print(state.current_airHumidity);
    mylcd.print("%");
    break;
  }
}
/*
 * DRAWS INTERFACE FOR THE DIFFERENT MENUS
 */
void drawSeleccion()
{

  switch(current_selectionstate)
  {
  case MENU:
    drawMenu();
    break;
  case FECHA:
    if(!isEditing)
      drawDate();
    else
      drawEditingDate(current_selectionDateState);
    break;
  case HORA:
    if(!isEditing)
      drawTime();
    else
      drawEditingTime(cTime);
    break;
  case CALIBRACION_SAT:
    drawCalibrationSat();
    break;
  case ABOUT:
    drawAbout();
    break;
  }

}


/*
 * DRAWS MENU
 */
void drawMenu()
{
  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(S_MAIN_MENU));
  int position=2;

  for(int i=current_menu; i<(current_menu+3) && i<MAXMENUITEMS;i++)
  {

    mylcd.setPosition(position,0);
    if(i==current_menu)
      mylcd.print("*");
    mylcd.print(translate(main_menu[i].label)); 
    position++;
    //  Serial.println(i);
  }
}

/**
 * draws the current state at LCD Screen
 * state: current state of sensors.
 */
void drawIrrigationState(State & state)
{
  mylcd.print("C");
  mylcd.print(current_rele+1);
  mylcd.print(":");
  drawRoleTitle(current_rele);
  if(state.field_capacity)
  { mylcd.print(" ");
    mylcd.print(translate(S_FC));
  }
  else
  {
   mylcd.print("   "); 
  }
  //Line2
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_S));
  if(state.moisture_MAX<10)
    mylcd.print("0");
  mylcd.print((int)state.moisture_MAX);
  mylcd.print(" ");
  mylcd.print(translate(S_MIN));
  if(state.moisture_MIN<10)
    mylcd.print("0");
  mylcd.print((int)state.moisture_MIN);
  mylcd.print(" ");
  mylcd.print("[");
  if(state.current_moisture<10)
    mylcd.print("0");
  mylcd.print((int)state.current_moisture);
  mylcd.print("");
  mylcd.print("]%");
  if(state.current_moisture<=99)
    mylcd.print(" ");



  //line3
  mylcd.setPosition(3,0);
  mylcd.print(translate(CICLO));
  int minutes=state.cicle_length_seconds/60;
  if(minutes<100)
    mylcd.print("0");
  if(minutes<10)
   mylcd.print("0");
  mylcd.print(minutes);
  mylcd.print("'");
  byte seconds= state.cicle_length_seconds%60;
  if(seconds<10)
    mylcd.print("0");
  mylcd.print(seconds);
  mylcd.print("''");
  mylcd.print("ON");
  if(current_sensorsvalues.cached_pump_percent<100)
   mylcd.print("0");
   if(current_sensorsvalues.cached_pump_percent<10)
    mylcd.print("0");
  mylcd.print(current_sensorsvalues.cached_pump_percent);
  mylcd.print("%");

  //line4
  mylcd.setPosition(4,0);
  mylcd.print(translate(ST_MAX));
  if(state.temps_max<10)
    mylcd.print("0");
  mylcd.print((int)state.temps_max);
  mylcd.print(" ");
  mylcd.print(translate(S_MIN));
  if(state.temps_min<10)
    mylcd.print("0");
  mylcd.print((int)state.temps_min);
  mylcd.print(" ");
  int currenttemp=state.current_temps;
  if(currenttemp>=0){
    mylcd.print("+"); 
  }else{
    if(currenttemp==-1000){mylcd.print("-");} 
  }
  if(currenttemp!=-1000){
  mylcd.print(currenttemp);
  }else{
   mylcd.print("--"); 
  }
  mylcd.print("C");
  //Serial.println(line1);

  //Serial.println(total);
  //LCD_Message(&mylcd,line1,line2,line3,"bb");
}


/*
 * ALWAYS LISTENS TO RELAYS STATES IN EACH ROLE
 */
void update_relay_state (void)
{
  byte i;
  boolean relaystate=false;
  for (i=0; i < MAX_RELAYS; i++){


    Relay rele = relay[i];
    switch (rele.role){
    case R_IRRIGATION:
     relaystate=false;
      if(!current_state.field_capacity){
        
      if (current_state.current_temps==-1000 ||(current_state.current_temps > current_state.temps_min && current_state.current_temps < current_state.temps_max))
      {
        if (current_state.current_moisture <= current_state.moisture_MIN )
        {


          relaystate=true;

        }
        else{
          if (rele.state==RELAY_ON && current_state.current_moisture <= current_state.moisture_MAX )
          {
            relaystate=true;
          }
        }
      }
      
     }else{
        relaystate=false; 
     }
      if(relaystate)
      {
        rele.state=RELAY_ON;
        relay_on(relay[i].gpio_pin);
      }
      else
      {
        rele.state=RELAY_OFF;
        relay_off(rele.gpio_pin);

      }
      relay[i]=rele;
     break;
     case R_HUMIDIFIER:
      
      if (current_state.current_airHumidity <= current_state.airHMIN)        
      {       
          relaystate=true;
       }  
       else{
         if(rele.state==RELAY_ON && current_state.current_airHumidity < current_state.airHRO)
         {
           relaystate=true;
         }
       }
 
      if(relaystate)
      {
        rele.state=RELAY_ON;
        relay_on(relay[i].gpio_pin);
      }
      else
      {
        rele.state=RELAY_OFF;
        relay_off(rele.gpio_pin);

      }
      relay[i]=rele;
      break;
    
    case R_COOLING:
      
      if (current_state.current_airTemperature > current_state.airTMAX)        
      {       
          relaystate=true;
       }  
       else{
         if(rele.state==RELAY_ON && current_state.current_airTemperature < current_state.airTAO)
         {
           relaystate=true;
         }
       }
 
      if(relaystate)
      {
        rele.state=RELAY_ON;
        relay_on(relay[i].gpio_pin);
      }
      else
      {
        rele.state=RELAY_OFF;
        relay_off(rele.gpio_pin);

      }
      relay[i]=rele;
      break;
    }
  }
   }


/*
 * CHECK IRRIGATION CICLE DURATION.
 * IRRIGATION IS CONFIGURABLE TO BE ON CONTINOUS MODE OR AT INTERVALS
 * BY INDICATING PUMP PERCENTAGE.
 * EXAMPLE: PUMP DURING FOUR MINUTES AT 50%. PUMPING WOULD BE ONE MINUTES ON & ONE MINUTES OFF, UNTIL COMPLETION OF THE CICLE OF FOUR MINUTES;
 */
boolean checkPumpCicle(boolean irrigating,tmElements_t time1,tmElements_t time2){
  int cicleLength=current_config.pump_cicle_length;
  int pumppercent= current_config.pump_percent;
  int totalIrrigationTime= (cicleLength*pumppercent)/100;
  int intervalTime=cicleLength/totalIrrigationTime;

  if(time_between(time1,time2)<intervalTime-1)
    irrigating= !irrigating;
  return irrigating;
} 
/*
 * DRAW TIME SELECTION MENU
 */
void drawTime()
{
  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(S_HOUR));
  mylcd.setPosition(2,0);
  if(editHours<10) mylcd.print("0");
  mylcd.print(editHours);
  mylcd.print(":");
  if(editMinutes<10) mylcd.print("0");
  mylcd.print(editMinutes);
  mylcd.setPosition(3,0);
  if(cTime==S_SAVETIME)
    mylcd.print("*");
  mylcd.print(translate(S_SAVE));
  mylcd.print(F(" "));
  mylcd.setPosition(4,0);
  if(cTime==S_BACKTIME)
    mylcd.print(F("*"));
  mylcd.print(translate(S_RETURN_TO));
  switch(cTime)
  {
  case S_SELECTHOURS:
    mylcd.setPosition(2,1);
    mylcd.boxCursorOn(); 
    break;
  case S_SELECTMINUTES:
    mylcd.setPosition(2,4);
    mylcd.boxCursorOn();
    break;
  default:
    mylcd.boxCursorOff();
    break;
  }
}
/*
 * DRAW DATE SELECTION MENU
 */
void drawDate()
{


  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(S_DATE));
  mylcd.setPosition(2,0);
  if(tm.Day<10) mylcd.print("0");
  mylcd.print(editDays);
  mylcd.print("/");
  if(tm.Month<10) mylcd.print("0");
  mylcd.print(editMonths);
  mylcd.print("/");
  mylcd.print(editYears);
  mylcd.setPosition(3,0);

  if(current_selectionDateState==SAVE)
  {
    mylcd.print("*");
  }
  mylcd.print(translate(S_SAVE));
  mylcd.print(" ");
  mylcd.setPosition(4,0);

  if(current_selectionDateState==BACK)
  {
    mylcd.print("*");
  }
  mylcd.print(translate(S_RETURN_TO));
  mylcd.print(" ");
  switch(current_selectionDateState)
  {
  case DAY:
    mylcd.setPosition(2,1);
    mylcd.boxCursorOn();
    break;
  case MONTH:
    mylcd.setPosition(2,4);
    mylcd.boxCursorOn();
    break;
  case YEAR:
    mylcd.setPosition(2,9);
    mylcd.boxCursorOn();
    break;
  default:
    mylcd.boxCursorOff();
    break;

  }


}
void drawEditingTime(byte currentTimeState)
{
  switch(currentTimeState)
  {

  case S_SELECTHOURS:
    mylcd.setPosition(1,0);
    printTitle(mylcd,translate(S_EDITHOUR));
    mylcd.setPosition(2,0);
    if(editHours<10)
      mylcd.print(F("0"));
    mylcd.print(editHours);
    mylcd.boxCursorOff();
    mylcd.setPosition(2,1);
    mylcd.boxCursorOn();
    break;
  case S_SELECTMINUTES:
    mylcd.setPosition(1,0);
    printTitle(mylcd,translate(S_EDITMINUTES));
    mylcd.setPosition(2,0);
    if(editMinutes<10)
      mylcd.print(F("0"));
    mylcd.print(editMinutes);
    mylcd.boxCursorOff();
    mylcd.setPosition(2,1);
    mylcd.boxCursorOn();
    break;

  }
}
void drawCalibrationSat()
{
  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(S_SATCALIBRATION));
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_CURRENTVALUE));
  mylcd.print(F(": "));
  mylcd.print(readFCapacityValue());
  actualizar_pantalla=true;

}
byte getDaysofMonth(byte month) {  

  switch(month){ 
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    return 31;
    break;
  case 2:
    return 2;
  case 4:
  case 6:
  case 9:
  case 11:
    return 30;
  default:
    return -1;
  }
}
void drawEditingDate(byte currentDateState)
{
  byte title;
  int data;
  switch(currentDateState)
  {

  case DAY:
    title=S_EDITDAY;
    data=editDays;
    break; 
  case MONTH:
    title=S_EDITMONTH;
    data=editMonths;
    break;
  case YEAR:
    title=S_EDITYEAR;
    data=editYears;
    break;
  }
  mylcd.boxCursorOff();
  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(title));
  mylcd.setPosition(2,0);
  if(data<10)
    mylcd.print(F("0"));
  mylcd.print(data);
  if(data>1000)
    mylcd.setPosition(2,3);
  else
    mylcd.setPosition(2,1);
  mylcd.boxCursorOn();
}

void static drawAbout()
{
  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(S_ABOUT));
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_SAC));
  mylcd.setPosition(3,0);
  mylcd.print(F("http://sacultivo.com"));
  mylcd.setPosition(4,0);
  mylcd.print(F("VERSION "));
  mylcd.print(VERSION);
} 
void static drawRoleMenu(State & state)
{
  mylcd.setPosition(1,0);
  if(tm.Hour<10) mylcd.print("0");
  mylcd.print(tm.Hour);
  mylcd.print(":");
  if(tm.Minute<10) mylcd.print("0");
  mylcd.print(tm.Minute);
  mylcd.print("  ");
  if(tm.Day<10) mylcd.print("0");
  mylcd.print(tm.Day);
  mylcd.print("/");
  if(tm.Month<10) mylcd.print("0");
  mylcd.print(tm.Month);
  mylcd.print("/");
  mylcd.print(tmYearToCalendar(tm.Year));
   for (int i=0; i<MAX_RELAYS;i++){
   mylcd.setPosition(i+2,0);
   mylcd.print("C");
   mylcd.print(i+1);
   mylcd.print(" ");
   drawRoleInfo(relay[i].role,state);
  }
}
void static drawSelectStatus(State & state)
{
  mylcd.setPosition(1,0);
  printTitle(mylcd,translate(S_EDITSTATE));
  //Line2
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_S));
  if(state.moisture_MAX<10)
    mylcd.print("0");
  mylcd.print((int)state.moisture_MAX);
  mylcd.print(" ");
  mylcd.print("MIN:");
  if(state.moisture_MIN<10)
    mylcd.print("0");
  mylcd.print((int)state.moisture_MIN);
  mylcd.print(" ");
  mylcd.print("[");
  if(state.current_moisture<10)
    mylcd.print("0");
  mylcd.print((int)state.current_moisture);
  mylcd.print("");
  mylcd.print("]%");
  if(state.moisture_target<=99)
    mylcd.print(" ");



  //line3
  mylcd.setPosition(3,0);
  mylcd.print(translate(CICLO));
  int minutes=state.cicle_length_seconds/60;
  if(minutes<100)
    mylcd.print("0");
  if(minutes<10)
   mylcd.print("0");
  mylcd.print(minutes);
  mylcd.print("'");
  byte seconds= state.cicle_length_seconds%60;
  if(seconds<10)
    mylcd.print("0");
  mylcd.print(seconds);
  mylcd.print("''");
  mylcd.print("ON");
  if(current_sensorsvalues.cached_pump_percent<100)
   mylcd.print("0");
   if(current_sensorsvalues.cached_pump_percent<10)
    mylcd.print("0");
  mylcd.print(current_sensorsvalues.cached_pump_percent);
  mylcd.print("%");

  //line4
  mylcd.setPosition(4,0);
  mylcd.print(translate(ST_MAX));
  if(state.temps_max<10)
    mylcd.print("0");
  mylcd.print((int)state.temps_max);
  mylcd.print(" ");
  mylcd.print("MIN:");
  if(state.temps_min<10)
    mylcd.print("0");
  mylcd.print((int)state.temps_min);
  mylcd.print(" ");
  int currenttemp=state.current_temps;
  if(currenttemp>=0){
    mylcd.print("+"); 
  }else{
    if(currenttemp==-1000)
       mylcd.print("-"); 
  }
  if(currenttemp!=-1000){
  mylcd.print(currenttemp);
  }else{
   mylcd.print("--"); 
  }
  mylcd.print("C");
  mylcd.boxCursorOff();
  mylcd.underlineCursorOff();
  switch(selectionStatus)
  {
  case S_HSO:

    if(!isEditing){
      mylcd.setPosition(2,2);
      mylcd.boxCursorOn(); 
    }
    else{
      mylcd.setPosition(2,5);
      mylcd.underlineCursorOn(); 
    }
    break;
  case S_HSMIN:
    if(!isEditing){
      mylcd.setPosition(2,9);
      mylcd.boxCursorOn(); 
    }
    else{
      mylcd.setPosition(2,12);
      mylcd.underlineCursorOn(); 
    }
    break;
  case S_TSMAX:
    if(!isEditing){
      mylcd.setPosition(4,4);
      mylcd.boxCursorOn(); 
    }
    else{
      mylcd.setPosition(4,7);
      mylcd.underlineCursorOn(); 
    }
    break;
  case S_TSMIN:
    if(!isEditing){
      mylcd.setPosition(4,11);
      mylcd.boxCursorOn(); 
    }
    else{
      mylcd.setPosition(4,14);
      mylcd.underlineCursorOn(); 
    }
    break;
  case S_PCICLE:
    if(!isEditing){
      mylcd.setPosition(3,4);
      mylcd.boxCursorOn(); 
    }
    else{
      mylcd.setPosition(3,9);
      mylcd.underlineCursorOn(); 
    }
    break;
  case S_PINTERVAL:
    if(!isEditing){
      mylcd.setPosition(3,15);
      mylcd.boxCursorOn(); 
    }
    else{
      mylcd.setPosition(3,19);
      mylcd.underlineCursorOn(); 
    }
    break;
  }
}
void drawHumidificationState(State & state)
{
    mylcd.setPosition(1,0);
    //printTitle(mylcd,translate(S_HUMIDIFIER));
    mylcd.print("C");
    mylcd.print(current_rele+1);
    mylcd.print(":");
    drawRoleTitle(current_rele);
    mylcd.setPosition(2,0);
    mylcd.print(translate(S_AIR_HUMIDITY));
    mylcd.setPosition(2,4);
    
    float airHRO = state.airHRO;
      
    mylcd.print(airHRO);
    mylcd.setPosition(2,7);

    mylcd.print(translate(S_MIN));
    mylcd.setPosition(2,10);
    float airHMIN = state.airHMIN;
    mylcd.print(airHMIN);
    mylcd.setPosition(2,12);
    mylcd.print("[");
    mylcd.print((float)state.current_airHumidity);
    mylcd.print("]");
    mylcd.print("%");
    
    mylcd.setPosition(3,0);
    mylcd.print(translate(CICLO));
    int minutes=state.cicle_length_seconds/60;
    if(minutes<100)
      mylcd.print("0");
    if(minutes<10)
     mylcd.print("0");
    mylcd.print(minutes);
    mylcd.print("'");
    byte seconds= state.cicle_length_seconds%60;
    if(seconds<10)
    mylcd.print("0");
    mylcd.print(seconds);
    mylcd.print("''");
    mylcd.print(translate(S_ON));
    mylcd.print("100");
    mylcd.print("%");
    
    
}
void drawRoleTitle(int current_rele)
{
  int role = relay[current_rele].role;
  switch(role)
 {
   case R_IRRIGATION:
    mylcd.print(translate(S_IRRIGATION));
   break;
   case R_HUMIDIFIER:
    mylcd.print(translate(S_HUMIDIFIER));
   break;
   case R_COOLING:
    mylcd.print(translate(S_COOLING));
   break;
 } 
}
void drawCoolingState(State & state)
{
  mylcd.setPosition(1,0);
  mylcd.print("C");
  mylcd.print(current_rele+1);
  mylcd.print(":");
  drawRoleTitle(current_rele);
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_AIR_TEMPERATURE));
  mylcd.print(":");
  mylcd.print((int)state.airTAO);
  mylcd.print(" ");
  mylcd.print(translate(S_TAMAX));
  mylcd.print((int)state.airTMAX);
  mylcd.print("[");
  mylcd.print((int)state.current_airTemperature);
  mylcd.print("]");
  mylcd.print("C");
  
}
void drawLigthState(State & state)
{
    mylcd.setPosition(1,0);
    printTitle(mylcd,translate(S_LIGHT));
    mylcd.setPosition(2,0);
    mylcd.print(translate(S_ON));
    mylcd.print("21:00H");
    mylcd.setPosition(3,0);
    mylcd.print(translate(S_OFF));
    mylcd.print("23:00H");
}


void handleEventSelectCoolingStatus(int event){
}
void handleEventSelectHumidifierStatus(int event){
}
