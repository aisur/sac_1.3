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
#define LCD_PIN 11
#define NUM_COLS 20
#define NUM_ROWS 4

void drawState(State & state);

SoftwareSerial SSerial(0,LCD_PIN);
SerLCD mylcd(SSerial,NUM_COLS,NUM_ROWS);


Configuration current_config;
tmElements_t tm;
cached_sensors current_sensorsvalues;
State current_state;
//--------------------------------------------------------------------------
//SACLCD saclcd(mylcd);

void setup()
{
  Serial.begin(9600);
  SSerial.begin(9600);
  mylcd.begin();

  
  
  // setupFlowRate();
 

}

void loop(){


  RTCread(tm);
  Serial.print("Bliblablu2");
  update_State(current_sensorsvalues,tm);


  State cstate=read_sensors(current_sensorsvalues);

  drawState(cstate);
  




  //LCD_Message(&mylcd,"",line2,"","Ultima Linea");
  delay(2000);

  // LCD_Clear(&mylcd);
}

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
  mylcd.print((int)state.moisture_MAX);
  mylcd.print("%");
  mylcd.print(translate(MIN));
  mylcd.print((int)state.moisture_MIN);
  mylcd.print("%");
  mylcd.print("[");
  mylcd.print((int)state.moisture_target);
  mylcd.print("%");
  mylcd.print("]");


 

  //line3
  mylcd.setPosition(3,0);
  mylcd.print(translate(CICLO+3));
  mylcd.print("00:00:00");
  mylcd.print("ON");
  mylcd.print("100%");
  
  //line4
   mylcd.setPosition(4,0);
  mylcd.print(translate(ST_MAX+2));
  mylcd.print((int)state.temps_max);
  mylcd.print("C ");
  mylcd.print(translate(MIN));
  mylcd.print((int)state.temps_min);
  mylcd.print("C ");
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


