#include <SoftwareSerial.h>
#include <SerLCD.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DS1307RTC.h>
#include <Wire.h>
#include <Time.h>
#include <time.h>

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
#define IDLE -1

#define TIEMPOACTUALIZAR 1*60*1000

void drawState(State & state);

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
};
/*
 * DIFFERENT STATES IN SELECTION MODE
 */
enum Seleccion_States
{
  MENU,
  IDIOMA,
  FECHA,
  HORA,
  CALIBRACION_SAT,
  RESET_CONFIG,
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
/*
 * MENU STRUCTURE 
 */
typedef struct MenuItem
{
  int label;
  int state; 
};

MenuItem main_menu[]={
  {
    S_LANGUAGE,IDIOMA  }
  ,{
    S_DATE,FECHA  }
  ,
  {
    S_HOUR,HORA  }
  ,{
    S_SATCALIBRATION,CALIBRACION_SAT  }
  ,
  {
    S_RESET,RESET_CONFIG  }
  ,{
    S_RETURN_TO,END_SELECTION  }
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
 * GLOBAL VARIABLES
 */
tmElements_t lastUpdate;
boolean irrigating;
byte current_mstate;
byte current_selectionstate;
byte current_selectionDateState;
byte cTime=S_SELECTHOURS;
byte cDate=DAY;
byte button_up_state=LOW;
byte button_down_state=LOW;
byte button_center_state=LOW;
byte center_pressed_state=0;
byte editHours;
byte editMinutes;
boolean isEditing;
byte editDays;
byte editMonths;
byte editYears;
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

  current_mstate=ESTADO;
  irrigating=false;
  current_selectionstate=MENU;
  if(!load_Settings(current_config)){
    current_config=reset_Settings();
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
}
/**
 * setup SAC Pins.
 */
void setup_pins(){
  pinMode(BUTTON_UP_PIN,INPUT);
  pinMode(BUTTON_DOWN_PIN,INPUT);
  pinMode(BUTTON_CENTER_PIN,INPUT);

  pinMode(SOIL_MOISTURE_POWER_PIN, OUTPUT);
  isEditing=false;
}
void loop(){


  RTCread(tm);

  //ALWAYS UPDATE SCREEN WHEN STATE CHANGES
  update_State(current_sensorsvalues,tm,current_config.calib_FCapacity);
  current_state=read_sensors(current_sensorsvalues);
  if(state_changed(current_state,previous_state) || time_between(lastUpdate,tm)>1){
    actualizar_pantalla=true; 
    previous_state=current_state;
    lastUpdate=tm;
  }
  else{
    actualizar_pantalla=false; 
  }



  int event=get_event();



  handleEvent(event);
  State cstate=read_sensors(current_sensorsvalues);
  drawUI(current_state);






  //Serial.println(freeRam());




  //LCD_Message(&mylcd,"",line2,"","Ultima Linea");
  delay(250);

  // LCD_Clear(&mylcd);
}
int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
/*Gets the current Button Event
 * returns: the button that is pressed.
 *  0: button UP is Pressed.
 *  1: button DOWN is Pressed.
 *  2: button CENTER is Pressed minor than 3 seconds.
 *  3: button CENTER is Pressed more than 3 seconds.
 */
int get_event(){
  if(button_up_pressed()==HIGH) return BUTTONUP;
  if(button_down_pressed()==HIGH) return BUTTONDOWN;
  return button_center_pressed();

}
/*
* Check if button up is pressed. The Event only ocurrs when the button is released; not
 * when button is pressed.
 returns HIGH when button Up is released or LOW otherwise.
 */
int button_up_pressed()
{
  byte current_state=digitalRead(BUTTON_UP_PIN);


  if(button_up_state==HIGH && current_state==LOW){
    button_up_state=current_state;
    return HIGH;
  }
  else{
    button_up_state=current_state;
    return LOW;
  }

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
    center_pressed_state++;
    return IDLE;

  }
  else{

    if(center_pressed_state<12 and center_pressed_state>0)//4 cicles per second and 3 secconds
    {
      center_pressed_state=0;
      return BUTTONCENTER;
    }
    else{
      if(center_pressed_state!=0){
        center_pressed_state=0;
        return BUTTONCENTERLONG;
      }
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
  switch(current_mstate)
  {
  case ESTADO:
    if(event==BUTTONCENTER){
      //Clear LCD Screen
      mylcd.clear();
      current_mstate=SELECCION;
      current_menu=0; 
      actualizar_pantalla=true;
    }
    break; 
  case SELECCION:

    handleEventSelection(event);

    break;
  }
}
/*
 * PRINTS MENU TITLE
 */
void printTitle(const char * message)
{
  mylcd.print("***");
  mylcd.print(message);
  mylcd.print("***");
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
  case IDIOMA:
    if(event == BUTTONCENTER)
    {
      mylcd.clear();
      active_language= select_language % MAX_LANGUAGE;
      current_selectionstate=MENU;
      actualizar_pantalla=true;
      current_config.active_languaje=active_language;
      store_Settings(current_config);
    }
    if(event== BUTTONUP)
    {
      select_language--; 
      actualizar_pantalla=true;            
    }
    if(event==BUTTONDOWN)
    {
      select_language++; 
      actualizar_pantalla=true;
    }
    break;

  case FECHA:
    {
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
      if(current_selectionDateState==BACK && event==BUTTONCENTER)
      {
        current_selectionstate=MENU;
        actualizar_pantalla=true;
        current_menu=0;
        mylcd.clear();
      }
    }
    break;
  case HORA:
    if(isEditing) 
      handleEventEditingHour(event);
    else
      handleEventSelectionHour(event);
    break;
  case CALIBRACION_SAT:
    if(event=BUTTONCENTER)
    {
      int calib=readFCapacityValue();
      current_config.calib_FCapacity=calib;
      store_Settings(current_config);
      current_selectionstate=END_SELECTION;
      mylcd.clear();
    }
    break;
  case RESET_CONFIG:

    current_config=reset_Settings();
    initializeGlobalVars();

  case END_SELECTION:
    current_mstate=ESTADO;
    current_selectionstate=MENU;
    actualizar_pantalla=true;
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


void handleEventEditingDate(int event)
{
  switch(current_selectionDateState)
  {
  case DAY:
    if(event==BUTTONDOWN){
      editDays++;
      editDays%=getDaysOfMonth(editMonths);
      editDays=(getDaysOfMonth(editMonths)>31)?editDays=1:editDays;
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editDays--;
      editDays%=getDaysOfMonth(editMonths);
      editDays=(getDaysOfMonth(editMonths)<1)?editDays=getDaysOfMonth(editMonths):editDays;
      actualizar_pantalla=true;
    }
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
    case YEAR:
    if(event==BUTTONDOWN){
      editYears++;
      editYears=(editYears>2100)?editYears=2000:editYears;  
      actualizar_pantalla=true;
    }
    if(event==BUTTONUP){
      editYears--;
      editYears=(editYears<2000)?editYears=2100:editYears;
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

    }
    actualizar_pantalla=false;
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
  case IDIOMA:
    drawSelectLanguage();
    break;
  case FECHA:
    drawDate();
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
  }

}
/*
 * DRAWS LANGUAGE SELECTION MENU
 */
void drawSelectLanguage()
{
  mylcd.setPosition(1,0);
  printTitle(translate(S_LANGUAGE));
  mylcd.setPosition(2,0);
  if(select_language==0)
    mylcd.print("*");
  mylcd.print(translate(S_ENGLISH));
  mylcd.setPosition(3,0);
  if(select_language==1)
    mylcd.print("*");
  mylcd.print(translate(S_SPANOL));
  mylcd.print(F(" "));
}
/*
 * DRAWS MENU
 */
void drawMenu()
{
  mylcd.setPosition(1,0);
  printTitle(translate(S_MAIN_MENU));
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
void drawState(State & state)
{
  //Line1
  long mil1=  millis();
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

  //Line2
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_S));
  if(state.moisture_MAX<10)
    mylcd.print("0");
  mylcd.print((int)state.moisture_MAX);
  mylcd.print("%");
  mylcd.print(translate(MIN));
  if(state.moisture_MIN<10)
    mylcd.print("0");
  mylcd.print((int)state.moisture_MIN);
  mylcd.print("%");
  mylcd.print("[");
  if(state.moisture_target<10)
    mylcd.print("0");
  mylcd.print((int)state.current_moisture);
  mylcd.print("%");
  mylcd.print("]");
  if(state.moisture_target<=99)
    mylcd.print(" ");



  //line3
  mylcd.setPosition(3,0);
  mylcd.print(translate(CICLO));
  mylcd.print("000'00''");
  mylcd.print("ON");
  mylcd.print("100%");

  //line4
  mylcd.setPosition(4,0);
  mylcd.print(translate(ST_MAX));
  if(state.temps_max<10)
    mylcd.print("0");
  mylcd.print((int)state.temps_max);
  mylcd.print(" ");
  mylcd.print(translate(MIN));
  if(state.temps_min<10)
    mylcd.print("0");
  mylcd.print((int)state.temps_min);
  mylcd.print(" ");
  int currenttemp=state.current_temps;
  if(currenttemp<0){
    mylcd.print("-"); 
  }
  else{
    mylcd.print("+");
  }
  mylcd.print(currenttemp);
  mylcd.print("C");
  //Serial.println(line1);
  long mil2=millis();
  long total=mil2-mil1;
  //Serial.println(total);
  //LCD_Message(&mylcd,line1,line2,line3,"bb");
}

/*
 * RELAY CONFIG
 *EACH RELAY USES A ROLE TO CONTROL IT'S FUNCTIONS
 */

Relay *find_relay (int role);
Relay relay[MAX_RELAYS]={
  {
    RELAY1_PIN    }
  ,{
    RELAY2_PIN    }
  ,{
    RELAY3_PIN    }
};
/*
 * ALWAYS LISTENS TO RELAYS STATES IN EACH ROLE
 */
void update_relay_state (void)
{
  int i;
  for (i=0; i < MAX_RELAYS; i++){

    if (relay[i].role != S_DISCONNECTED)
    {
      Relay rele = relay[i];
      switch (rele.role){
      case S_IRRIGATION:
        {
          if (current_state.current_moisture > current_state.moisture_MIN && current_state.current_moisture < current_state.moisture_target)
          {
            if (current_state.current_temps > current_state.temps_min && current_state.current_temps < current_state.temps_max)
            {
              if (!current_state.field_capacity)
              {
                //if(checkPumpCicle(irrigating,current_sensorsvalues.cached_lastWaterEvent,tm)){
                relay_on(&rele); 
                // }else{

                // }
              }
              else{
                relay_off(&rele); 
              } 
            }
          }
        }
      }
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
  printTitle(translate(S_HOUR));
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
  printTitle(translate(S_DATE));
  mylcd.setPosition(2,0);
  if(tm.Day<10) mylcd.print("0");
  mylcd.print(tm.Day);
  mylcd.print("/");
  if(tm.Month<10) mylcd.print("0");
  mylcd.print(tm.Month);
  mylcd.print("/");
  mylcd.print(tmYearToCalendar(tm.Year));
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
    printTitle(translate(S_EDITHOUR));
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
    printTitle(translate(S_EDITMINUTES));
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
  printTitle(translate(S_CALIBRATE_MOIST));
  mylcd.setPosition(2,0);
  mylcd.print(translate(S_CURRENTVALUE));
  mylcd.print(F(": "));
  mylcd.print(readFCapacityValue());
  actualizar_pantalla=true;
}
void drawEditingDate(byte currentDateState)
{
  switch (currentDateState)
  {
    case DAY:
      mylcd.setPosition(1,0);
      printTitle(translate(S_EDITDAY));
      mylcd.setPosition(2,0);
      if(editDays<10)
         mylcd.print(F("0"));
      mylcd.print(editDays);
      mylcd.boxCursorOff();
      mylcd.setPosition(2,1);
      mylcd.boxCursorOn();
      break;
    case MONTH:
      mylcd.setPosition(1,0);
      printTitle(translate(S_EDITMONTH));
      mylcd.setPosition(2,0);
    if(editMonths<10)
      mylcd.print(F("0"));
      mylcd.print(editMonths);
      mylcd.boxCursorOff();
      mylcd.setPosition(2,1);
      mylcd.boxCursorOn();
      break;
    case YEAR:
       mylcd.setPosition(1,0);
      printTitle(translate(S_EDITYEAR));
    mylcd.setPosition(2,0);
    if(editYears<10)
      mylcd.print(F("0"));
    mylcd.print(editYears);
    mylcd.boxCursorOff();
    mylcd.setPosition(2,1);
    mylcd.boxCursorOn();
    break;
  }
byte getDaysOfMonth(byte month)
{
  byte nDays;
  switch(month)
  {
    case 1:
    case 3:
    case 5:
    case 7: 
    case 8:
    case 10:
    case 12:
    nDays = 31;
    break;
    case 2:
    nDays=28;
    break;
    case 4:
    case 6:
    case 9:
    case 11:
    nDays=30;
    break;
    default:
    nDays =-1;
    break;
  }
  return nDays;
}
}

