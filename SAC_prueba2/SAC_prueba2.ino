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

#define LCD_PIN 11
#define NUM_COLS 20
#define NUM_ROWS 4

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
enum States
{
  INICIO,
  SELECTIDIOMA,
  CALIB_SAT,
  ESTADO,
  EDICION,
  SELECCION,
};

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

typedef struct MenuItem
{
   int label;
   int state; 
};

MenuItem main_menu[]={{S_LANGUAGE,IDIOMA},{S_DATE,FECHA},
                      {S_HOUR,HORA},{S_SATCALIBRATION,CALIBRACION_SAT},
                      {S_RESET,RESET_CONFIG},{S_RETURN_TO,END_SELECTION}};
int current_menu;

SoftwareSerial SSerial(0,LCD_PIN);
SerLCD mylcd(SSerial,NUM_COLS,NUM_ROWS);

boolean actualizar_pantalla;
Configuration current_config;
tmElements_t tm;
cached_sensors current_sensorsvalues;
State current_state;
State previous_state;
tmElements_t lastUpdate;
boolean irrigating;
int current_mstate;
int current_selectionstate;

byte button_up_state=LOW;
byte button_down_state=LOW;
byte button_center_state=LOW;
byte center_pressed_state=0;
//--------------------------------------------------------------------------
//SACLCD saclcd(mylcd);

void setup()
{
  Serial.begin(9600);
  SSerial.begin(9600);
  mylcd.begin();
  setup_pings();
  actualizar_pantalla=true;
  // setupFlowRate();
  current_mstate=ESTADO;
  irrigating=false;
  current_selectionstate=MENU;
}
/**
 * setup SAC Pins.
 */
void setup_pings(){
  pinMode(BUTTON_UP_PIN,INPUT);
  pinMode(BUTTON_DOWN_PIN,INPUT);
  pinMode(BUTTON_CENTER_PIN,INPUT);

  pinMode(SOIL_MOISTURE_POWER_PIN, OUTPUT);
}
void loop(){


  RTCread(tm);

  update_State(current_sensorsvalues,tm);
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
  drawUI(current_state);


  Serial.print("boton ");
  Serial.println(event);




  //LCD_Message(&mylcd,"",line2,"","Ultima Linea");
  delay(250);

  // LCD_Clear(&mylcd);
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
    }
    break; 
  case SELECCION:
    
      handleEventSelection(event);
    
    
  }
}
void handleEventSelection(int event)
{
    switch(current_selectionstate)
    {
       case MENU:
         if(event == BUTTONCENTER)
         {
            mylcd.clear();
            current_selectionstate=main_menu[current_menu].state;
         }
         break;
    }
}
void drawUI(State & state){
  switch(current_mstate)
  {
  case SELECCION:
    drawSeleccion();
    break;
  case  ESTADO:
    if(actualizar_pantalla)
      drawState(state);
    break;

  }
}


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
  }
 
}
void drawSelectLanguage()
{
  mylcd.setPosition(1,0);
  mylcd.print("IDIOMA");
}
void drawMenu()
{
   mylcd.setPosition(1,0);
  mylcd.print(translate(S_MAIN_MENU));
  int position=2;
  Serial.print("Menu");
  Serial.println(current_menu);
  for(int i=current_menu; i<(current_menu+3);i++)
  {
     mylcd.setPosition(position,0);
     mylcd.print(translate(main_menu[i].label)); 
     position++;
     Serial.println(i);
  }
}
/**
 * draw the current state at LCD Screen
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
  Serial.println(total);
  //LCD_Message(&mylcd,line1,line2,line3,"bb");
}



Relay *find_relay (int role);
Relay relay[MAX_RELAYS]={
  {
    RELAY1_PIN  }
  ,{
    RELAY2_PIN  }
  ,{
    RELAY3_PIN  }
};

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
                if(checkPumpCicle(irrigating,current_sensorsvalues.cached_lastWaterEvent,tm)){
                  relay_on(&rele); 
                }else{
                  relay_off(&rele); 
                }
              } 
            }
          }
        }
      }
    }
  }
}
boolean checkPumpCicle(boolean irrigating,tmElements_t time1,tmElements_t time2){
    int cicleLength=current_config.pump_cicle_length;
    int pumppercent= current_config.pump_percent;
    int totalIrrigationTime= (cicleLength*pumppercent)/100;
    int intervalTime=cicleLength/totalIrrigationTime;
    
   if(time_between(time1,time2)<intervalTime-1)
     irrigating= !irrigating;
     return irrigating;
} 

