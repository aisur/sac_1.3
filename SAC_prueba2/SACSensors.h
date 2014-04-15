#include <Time.h>
#include <OneWire.h>
#define DS19S20_PIN A1

/*
 * MOISURE POWER PIN
 */
#define SOIL_MOISTURE_POWER_PIN 3

/*
 * MOISURE PIN
 */
#define MOISTURE_PIN A3

/*
 * WATER TANK PIN
 */
#define WTS_PIN A2
#define WFL_PIN 6
#define FC_PIN A2
volatile int NbTopsFan;
int Calc;

boolean readFieldCapacity(int fcapacity_calib){
        int Fcapacity= analogRead(FC_PIN);
         
         digitalWrite(SOIL_MOISTURE_POWER_PIN, HIGH);
  
  int cached_fc = analogRead(FC_PIN);
  digitalWrite(SOIL_MOISTURE_POWER_PIN, LOW);
  return        map(cached_fc,0,fcapacity_calib,0,1);
}

void npm();
/*
* this struct store the last cached data sensor.
*/
typedef struct {
  float cached_temperature;//Last Temperature
  float cached_humidity;//Last Air Humidity
  int cached_waterlevel;//Last WaterLevel
  float cached_flowvolume; //Water Flow
  float cached_tempmin;//Last Minimum Temperature
  float cached_tempmax;//Last Maximum Temperature
  float cached_moisture;//Last Soil Moisture
  float cached_minmoisture;//Last Minimum Temperature
  float cached_maxmoisture;//Last Maximum Temperature
  int cached_waterFlowdiameter;//Water Flow Diameter
  boolean cached_fieldCapacity;//last result for the field capacity
  tmElements_t cached_lastWaterEvent;//Last Time and Date for Pumping.
} cached_sensors;
/*
 * This Struct store all the information for the current state.
 */
typedef struct {

	/**
	 * the moisture target
	 */
	float moisture_target;
	/**
	 * current moisture_MIN
	 */
	float moisture_MIN;
	/**
	 * current moisture_MIN
	 */
	float moisture_MAX;
	/**
	 * current moisture
	 */
	float current_moisture;
	/*
	 * Max Soil Temperature
	 */
	float temps_max;
	/*
	 * Min Soil Temperature
	 */
	float temps_min;
	/*
	 * Current Soil Temperature
	 */
	float current_temps;
	/*
	 * Current Water Consumption in m3
	 */
	float consumption;

	boolean field_capacity;

}State;

float read_SoilTemp()
{
  OneWire ds(DS19S20_PIN);  // on pin A1
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;
}

 int read_moisture(){
   
  digitalWrite(SOIL_MOISTURE_POWER_PIN, HIGH);
  
  int cached_moisture = analogRead(MOISTURE_PIN);
  digitalWrite(SOIL_MOISTURE_POWER_PIN, LOW);
  cached_moisture=  map(cached_moisture,0,602,0,100);
  return cached_moisture;
 }
 
 boolean getWaterLevel(){

	int WaterLevel=analogRead(WTS_PIN);
	int WaterUp=map(WaterLevel,0,602,0,1);
	//Note When We Want analog Water results, change map function
	return WaterUp>0;
}
 void setupFlowRate()
 {
   NbTopsFan=0;
   pinMode(WFL_PIN, INPUT);
   attachInterrupt(0,npm,RISING);
 }
 
 void npm()
{
  NbTopsFan++;
}
float getWaterFlowRate ()
{
    
  sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
  cli();      //Disable interrupts
  Calc = (NbTopsFan * 7 / 60); //(Pulse frequency x 60) / 7Q, = flow rate in L/min
  return Calc;
  /*  Serial.print (Calc, DEC); //Prints the number calculated above*/
}

void update_State(cached_sensors & last_values,tmElements_t current_event,int FCapacityCalib)
{
            
            float curr_moisture=read_moisture();
	    float curr_temps= read_SoilTemp();
	    float curr_flowrate=0.0;//getWaterFlowRate();
  
            last_values.cached_moisture=curr_moisture;
	    last_values.cached_temperature=curr_temps;
	    last_values.cached_waterlevel = 0.0;//getWaterLevel(); // Boolean indicates if we have water or not.
	    last_values.cached_flowvolume+=curr_flowrate/60000;//FlowRate(L/m) to FlowRate(m3/s).
	    last_values.cached_fieldCapacity=readFieldCapacity(FCapacityCalib);
}

State read_sensors(cached_sensors & last_values)
{
 State current_state;


	/* set the state */
	current_state.moisture_MAX=last_values.cached_maxmoisture;
	current_state.moisture_MIN=last_values.cached_minmoisture;

        current_state.current_moisture=last_values.cached_moisture;
	current_state.consumption=last_values.cached_flowvolume;
	current_state.current_temps=last_values.cached_temperature;
	current_state.moisture_target=last_values.cached_moisture;
	current_state.temps_max=last_values.cached_tempmax;
	current_state.temps_min=last_values.cached_tempmin;
        current_state.current_temps=last_values.cached_temperature;
	current_state.field_capacity=last_values.cached_fieldCapacity;
	return current_state; 
  
}

cached_sensors initSensorsCache(){
  cached_sensors current_sensors;
   current_sensors.cached_moisture=0.0;
   current_sensors.cached_temperature=0.0;
   current_sensors.cached_waterlevel = 0.0;//getWaterLevel(); // Boolean indicates if we have water or not.
   current_sensors.cached_flowvolume+=0.0;//FlowRate(L/m) to FlowRate(m3/s).
   current_sensors.cached_fieldCapacity=true;
   current_sensors.cached_humidity=0;
   current_sensors.cached_tempmin=0;
   current_sensors.cached_tempmax=0;
   current_sensors.cached_minmoisture=0;
   current_sensors.cached_maxmoisture=0;
}
int readFCapacityValue()
{
  digitalWrite(SOIL_MOISTURE_POWER_PIN, HIGH);
  
  int cached_fc = analogRead(FC_PIN);
  digitalWrite(SOIL_MOISTURE_POWER_PIN, LOW);
  return cached_fc;
}
boolean state_changed(State state1,State state2)
{
   if(state1.current_moisture!= state2.current_moisture ) return true;
   if(state1.current_temps!= state2.current_temps ) return true;
   if(state1.field_capacity!= state2.field_capacity ) return true;
   return false;
}
