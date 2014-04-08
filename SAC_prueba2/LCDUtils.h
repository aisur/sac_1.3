
#include <SoftwareSerial.h>
#include <SerLCD.h>

/****************************************************************
LCDUtils: this file contains all the information about the use of the
LCD Screen Used in the project:
  http://saccultivo.com
  
  This file is prepared by default for 20x4 Serial LCD Screen.
  
  Version: 1.1(b)
  Author: Victor Suarez Garcia<suarez.garcia.victor@gmail.com>
  Co-Author: David Cuevas Lopez<mr.cavern@gmail.com>
  
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
 * 0.1. Initial Version
 * 0.2. Changed the Library version.
 * 0.3. Changed the functions for use the SerialLCD Library.
 * Current Version: 0.3.
*************************************************************************/

void LCD_Message(SerLCD* lcd,char * line1,char* line2,char * line3,char* line4);
void LCD_print(SerLCD* lcd,char * message,int col, int row);
void LCD_Clear(SerLCD* lcd);
void LCD_ClearLine(SerLCD* lcd,int lineNo);

/*
* Show until 4 messages in the LCD Screen
* Parameters
* lcd: pointer to the lcd variable for send the information to the LCD Screen.
* line1: string for the first line
* line2: string for the second line
* line3: string for the third line
* line4: string for the fourth line
*/
void LCD_Message(SerLCD* lcd,char * line1,char* line2,char * line3,char* line4){
  
  
        //LCD_ClearLine(lcd,1);
	lcd->setPosition(1, 0);
	lcd->print(line1);
        //LCD_ClearLine(lcd,2);
	lcd->setPosition(2, 0);
	if(line2)
		lcd->print(line2);
        //LCD_ClearLine(lcd,3);
	lcd->setPosition(3,0);
	if(line3)
		lcd->print(line3);
        //LCD_ClearLine(lcd,4);
	lcd->setPosition(4,0);
	if(line4)
		lcd->print(line4);
  
}
/*
* Prints a Message in one specific position.
* Parameters:
* lcd: pointer to the lcd variable for send the information to the LCD Screen.
* message: message to Show.
* col: specific Column for show the Message.
* row: specific Row for show the Message.
*/
void LCD_print(SerLCD* lcd,char * message,int col, int row)
{
   lcd->setPosition(col,row);
   lcd->print(message); 
}
/*
* Clear the LCD Screen
* Parameters
* lcd: pointer to the lcd variable for send the information to the LCD Screen.
*/
void LCD_Clear(SerLCD* lcd)
{
  lcd->clear(); 
}
void LCD_ClearLine(SerLCD* lcd,int lineNo)
{
 String blanckLine="                    ";
 lcd->setPosition(0,lineNo%4);
 lcd->print(blanckLine); 
}
