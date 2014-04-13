/*
 * languages.h : File That contains all the Language Options for the Project SAC.
 *  In this file contains all the Messages for Print in screen in 2 Languages.
 *
 *  Created on: 16/03/2014
 *      Author: David Cuevas <mr.cavern@gmail.com>
 *      Collaborator: Victor Suarez<suarez.garcia.victor@gmail.com>
 *
 *   SAC: Version 1.1.
 */
int active_language=1;


enum {
  S_EMPTY=0,
  S_WA,
  S_V,
  S_S,
  S_H,
  S_L,
  S_A,
  S_PU,
  S_LANGUAGE,
  S_ENGLISH,
  S_ENABLED,
  S_DISABLED,
  S_TIME,
  S_SOIL_MOISTURE,
  S_CALIBRATE_MOIST,
  S_DISCONNECTED,
  S_IRRIGATION,
  S_HEATING,
  S_COOLING,
  S_LIGHT,
  S_VENTILATION,
  S_HUMIDIFIER,
  S_ALARM,
  S_RELAY1,
  S_RELAY2,
  S_RELAY3,
  S_AIR_HUMIDITY,
  S_AIR_TEMPERATURE,
  S_IRRIGATION_CYCLE,
  S_LENGTH_SEC,
  S_RETURN_TO,
  S_MAIN_MENU,
  S_SETUP,
  S_RELAYS,
  S_RESET,
  S_ON,
  S_START,
  S_DURATION,
  S_CONFIG,
  S_RANGE,
  S_LOG,
  MIN,
  ST_MAX,
  CONSUMPTION,
  CICLO,
  S_DATE,
  S_HOUR,
  S_SATCALIBRATION,
  S_SPANOL,
  S_SAVE,
  S_EDITDAY,
  S_EDITHOUR
};

#define MAX_LANGUAGE 2
typedef struct TranslatedString {
  const char *languages[MAX_LANGUAGE];
} TranslatedString;

TranslatedString string_db[]={
  /* for languages missing translations, english will be used instead */
  {{"",""}},
  {{"  F.C.","C.C."}},
  {{"E ","E "}},
  {{"SOIL","HSO:"}},
  {{"H ","H "}},
  {{"L ","L "}},
  {{"A ","A "}},
  {{"PU ","RIEGO"}},
  {{"LANGUAGE","IDIOMA"}},
  {{"ENGLISH",         "INGLES"}},
  {{"<enabled>",       "<activar>"}},
  {{"<disabled>",      "<inhabilitado>"}},
  {{"TIME",            "HORA"}},
  {{"SOIL MOISTURE",   "HUMEDAD SUELO"}},
  {{"CALIBRATE SAT.","CALIBRACION SAT."}},
  {{"OFF",             "APAGADO"}},
  {{"WATERING",      "RIEGO"}},
  {{"HEATING",         "CALEFACCION",}},
  {{"COOLING",         "REFRIGERACION"}},
  {{"LIGHT",           "ILUMINACION"}},
  {{"AIR EXTRACTION",  "EXTRAC.HUMEDAD"}},
  {{"HUMIDIFIER",      "HUMIDIFICACION"}},
  {{"alarm",           "Alarm"}},
  {{"OUTPUT 1",        "SALIDA 1"}},
  {{"OUTPUT 2",        "SALIDA 2"}},
  {{"OUTPUT 3",        "SALIDA 3"}},
  {{"AIR HUMIDITY",    "HUMEDAD AIRE"}},
  {{"TEMPERATURE",     "TEMPERATURA"}},

  {{"WATERING",      "RIEGO"}},
  {{"CICLES:",  	"CICLOS:"}},

  {{"RETURN TO",       "VOLVER"}},
  {{"MAIN MENU",       "MENU PRINCIPAL"}},
  {{"Setup...",        "Configuracion.."}},
  {{"OUTPUTS",          "SALIDAS"}},
  {{"Reset",           "RESET"}},
  {{"ON",              "ENCENDIDO"}},
  {{"START",           "COMIENZA"}},
  {{"DURATION",        "DURACION"}},
  {{"CONFIGURATION",   "CONFIGURACION"}},
  {{"Hysteresis: ",  }},
  {{"Log"}},
  {{"MIN:","MIN:"}},
  {{"STMAX:","TSMAX:"}},
  {{"CONSUMPTION:","CONSUMO:"}},
  {{"CICLE:","CICLO:"}},
  {{"DATE","FECHA"}},
  {{"HOUR","HORA"}},
  {{"SAT CALIBRATION","CALIBRATION SAT"}},
  {{"SPANISH","ESPANOL"}},
  {{"SAVE","GUARDAR"}},
  {{"EDIT DAY","EDITAR DIA"}},
  {{"EDIT HOUR","EDITAR HORA"}}

};



/* returns a translated string; if no translation found - return the original
 * string.
 */
static const char *translate(int stringno)
{
  if (active_language < 0)
    active_language = 0;
  if (active_language >= MAX_LANGUAGE)
    active_language = MAX_LANGUAGE-1;

  if (string_db[stringno].languages[active_language])
    return string_db[stringno].languages[active_language];
  else
    return string_db[stringno].languages[0];
}
