 /*
File: commands.h
Purpose: Header file for commands.c

Author: Erika McCluskey, Hayden Jin
*/

#include "util.h"
#include <string.h>

 
void turn_on_green_LED(void);
void turn_off_green_LED(void);
void turn_on_red_LED(void);
void turn_off_red_LED(void);
void flash_LED(char colour, int howManyTimes, int delay);
bool add_button_pressed(void);
bool verify_button_pressed(void);
bool reset_button_pressed(void);
char state_of_buttons(void);	
void authorized(void);
void device_ready(void);
uint32_t readSensorData(void);