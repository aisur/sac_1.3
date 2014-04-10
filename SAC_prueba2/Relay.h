/*
 * Relay.h: This file contains all the configuration for the relays and their states
 * and different roles.
 *
 *  Created on: 10/04/2014
 *      Author: David Cuevas
 *      Co-Author: victor suarez
 * * This library is free software; you can redistribute it and/or
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
 * Vesion History:
 * 0.1. Initial Version 
 * 
 * Current Version: 0.1
 */
 

#include "Arduino.h"

#define MAX_RELAYS 4
typedef enum _RelayState RelayState;
typedef struct _Relay Relay;

  enum _RelayState {
    RELAY_OFF,
    RELAY_ON,
    RELAY_WAITING  /* we're off duty in our cycle */
 };
  
  struct _Relay {
  int gpio_pin;
  int role;
  int state;
 };
 


void relay_on (Relay *relay)
{
  if (relay->state == RELAY_ON)
    return;
  relay->state = RELAY_ON;


  if (relay->gpio_pin)
    digitalWrite(relay->gpio_pin, HIGH);

}

void relay_wait (Relay *relay)
{
  if (relay->state == RELAY_WAITING)
    return;
  relay->state = RELAY_WAITING;

  digitalWrite(relay->gpio_pin, LOW);

}

void relay_off (Relay *relay)
{
  if (relay->state == RELAY_OFF)
    return;
  relay->state = RELAY_OFF;

  digitalWrite(relay->gpio_pin, LOW);

}

int roles[] = {
  S_DISCONNECTED,
  S_IRRIGATION,
  S_HUMIDIFIER,
  S_VENTILATION,
  S_HEATING,
  S_COOLING,
  S_LIGHT,

/*  S_ALARM,*/
/*  S_ON*/
};
