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

void drawState(State & state);

SoftwareSerial SSerial(0,LCD_PIN);
SerLCD mylcd(SSerial,NUM_COLS,NUM_ROWS);


Configuration current_config;
tmElements_t tm;
cached_sensors current_sensorsvalues;
State current_state;

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
  
  
  // setupFlowRate();
 

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

  int event=get_event();
  
  Serial.print("Boton :");
  Serial.println(event);
  
  State cstate=read_sensors(current_sensorsvalues);

  drawState(cstate);
  




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
  }else{
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
  }else{
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
  }else{
    if(center_pressed_state<3 and center_pressed_state>0)//4 cicles per second and 3 secconds
    {
      center_pressed_state=0;
      return BUTTONCENTER;
    }else{
      if(center_pressed_state!=0){
      center_pressed_state=0;
      return BUTTONCENTERLONG;
      }
    }
  }
    return IDLE;
  
}

/**
* draw the current state at LCD Screen
* state: current state of sensors.
*/
void drawState(State & state)
{
  //Line1
  
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
  mylcd.print((int)state.moisture_target);
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
  }else{
    mylcd.print("+");
  }
  mylcd.print(currenttemp);
  mylcd.print("C");
  //Serial.println(line1);
  
  //LCD_Message(&mylcd,line1,line2,line3,"bb");
}



Relay *find_relay (int role);
Relay relay[MAX_RELAYS]={{RELAY1_PIN},{RELAY2_PIN},{RELAY3_PIN}};

void update_relay_state (void)
{
 int i;
for (i=0; i < MAX_RELAYS; i++){
  
    if (relay[i].role != DISCONNECTED)
      {
       Relay rele = relay[i];
       switch (relay[i].role){
         case IRRIGATION:
           {
             if (current_state.current_moisture > moisture_min && current_state.current_moisture < moisture_target)
             {
               if (current_state.current_temps > temps_min && current_state.current_temps < temps_max)
               {
                 if (!current_state.field_capacity)
                {
                  
                } 
               }
             }
           }
       }
      }
}
      
