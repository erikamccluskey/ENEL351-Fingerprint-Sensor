/*
Functions for the LCD written with the help of Tessa Herzberger's code

File: LCD.h
Purpose: Header file for LCD.c 

Author: Erika McCluskey, Hayden Jin
*/

#include "util.h"
#include <stdbool.h>

void stringToLCD(char * word);
void dataToLCD (uint8_t data);
void commandToLCD(uint8_t data);
void lcd_init();
void LCD_show(char * word);

#define LCD_CM_ENA 0x00050002 
#define LCD_CM_DIS 0x00070000 
#define LCD_DM_ENA 0x00040003 
#define LCD_DM_DIS 0x00060001 

#define LCD_8B2L 0x38 // ; Enable 8 bit data, 2 display lines
#define LCD_DCB 0x0F // ; Enable Display, Cursor, Blink
#define LCD_MCR 0x06 // ; Set Move Cursor Right
#define LCD_CLR 0x01 // ; Home and clear LCD
#define LCD_LN1 0x80 // ;Set DDRAM to start of line 1
#define LCD_LN2 0xC0 // ; Set DDRAM to start of line 2


#define	eSet()		GPIOB->BSRR  |= GPIO_BSRR_BS1
#define	eClear()	GPIOB->BSRR  |= GPIO_BSRR_BR1