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
  S_S=0,
  S_PU,
  S_LANGUAGE,
  S_ENGLISH,
  S_TIME,
  S_SOIL_MOISTURE,
  S_DISCONNECTED,
  S_IRRIGATION,
  S_HEATING,
  S_COOLING,
  S_LIGHT,
  S_VENTILATION,
  S_HUMIDIFIER,
  S_AIR_HUMIDITY,
  S_AIR_TEMPERATURE,


  S_RETURN_TO,
  S_MAIN_MENU,

  S_RELAYS,
  S_RESET,
  S_ON,
  S_START,
  S_OFF,
  S_DURATION,

  S_RANGE,
  
  ST_MAX,
  CONSUMPTION,
  CICLO,
  S_DATE,
  S_HOUR,
  S_SATCALIBRATION,
  S_SPANOL,
  S_SAVE,
  S_EDITDAY,
  S_EDITHOUR,
  S_EDITMINUTES,
  S_CURRENTVALUE,
  S_EDITMONTH,
  S_EDITYEAR,
  S_ABOUT,
  S_SAC,
  S_EDITSTATE,
  S_FC,
  S_MIN
};

#define MAX_LANGUAGE 1
typedef struct TranslatedString {
 PROGMEM const char *languages[MAX_LANGUAGE];
} TranslatedString;

TranslatedString string_db[]={
  /* for languages missing translations, english will be used instead */
  {{"HSO:"}},
  {{"RIEGO"}},
  {{"IDIOMA"}},
  {{"INGLES"}},
  {{"HORA"}},
  {{"HUMEDAD SUELO"}},
  {{"APAGADO"}},
  {{"RIEGO"}},
  {{"CALEFACCION",}},
  {{"REFRIGERACION"}},
  {{"ILUMINACION"}},
  {{"DESHUMIDIFICACION"}},
  {{"HUMIDIFICACION"}},
  {{"HRO"}},
  {{"TAO"}},


  {{"VOLVER"}},
  {{"MENU PRINCIPAL"}},

  {{"SALIDAS"}},
  {{"RESET"}},
  {{"ON:"}},
  {{"INICIO:"}},
  {{"OFF:"}},
  {{"DURACION"}},

  {{" "  }},

  {{"TSMAX:"}},
  {{"CONSUMO:"}},
  {{"CICLO:"}},
  {{"FECHA"}},
  {{"HORA"}},
  {{"CALIBRATION SAT"}},
  {{"ESPANOL"}},
  {{"GUARDAR"}},
  {{"EDITAR DIA"}},
  {{"EDITAR HORA"}},
  {{"EDITAR MINUTOS"}},
  {{"VALOR ACTUAL"}},
  {{"EDITAR MES"}},
  {{"EDITAR ANO"}},
  {{"ACERCA DE"}},
  {{"DESARROLLO POR AISUR"}},
  {{"EDITAR ESTADO"}},
  {{"CC"}},
  {{"MIN"}}
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
