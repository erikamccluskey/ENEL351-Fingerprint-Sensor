/*
Functions for the LCD written with the help of Tessa Herzberger's code

File: LCD.c
Purpose: functions related to the LCD (initialization, data send)

Author: Erika McCluskey, Hayden Jin
*/

#include "lcd.h"
 #include "stm32f10x.h"
 
void ePulse (void)
{
	delay(60000) ;	
	eSet () ;
	delay(60000) ;
	eClear () ;
}

void commandToLCD(uint8_t data)
{
	//This function commands the LCD screen to perform a task.
	
	//Set enable, .read, write and reset bits.
	GPIOB->BSRR = LCD_CM_ENA;
	
	//GPIOB->BSRR &= ~GPIO_BSRR_BR1;
	
	//Clear the last 8 bits.
	GPIOC->ODR &= 0xFF00;
	//keep the bits that are the same as the data bits.
	GPIOC->ODR |= data;
	
		//call the delay function.
	delay(7000);
	
	//Reset enable, read, write and reset bits.
	GPIOB->BSRR = LCD_CM_DIS;
	
	//call the delay function.
	delay(7000);
	
	ePulse();
}


void lcd_init()
{
  //Enable peripheral clocks for various ports and subsystems
	RCC->APB2ENR |=  RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPBEN ;
	
	//Initialize the inputs on GPIOC and GPIOB
//	ePulse();
	GPIOC->CRL = 0x33333333;
//	ePulse();
	GPIOB->CRL = 0x00000333;
	
	//Define the screen to be 8 bits and two lines.
	commandToLCD (LCD_8B2L);
	delay (2000000);
	//Define the screen to be 8 bits and two lines.
	commandToLCD (LCD_8B2L);
	delay (2000000);
	//Define the screen to be 8 bits and two lines.
	commandToLCD (LCD_8B2L);
	delay (2000000);
  //Define the screen to be 8 bits and two lines.
	commandToLCD (LCD_8B2L);
	delay (2000000);
	
	//Enable the display curser blank.
	commandToLCD (LCD_DCB);
	delay (2000000);
	//Move the curser to the right.
	commandToLCD (LCD_MCR);
	delay (2000000);
	//Clear the LCD screen.
	commandToLCD (LCD_CLR);
	delay (2000000);
	//Put the curser on the first line.
	commandToLCD (LCD_LN1);
	delay (2000000);
}


void dataToLCD(uint8_t data)
{
	//This function writes data the LCD screen.
	
	//Set enable, read, write and reset bits.
	GPIOB->BSRR = LCD_DM_ENA;
	//Clear the last 8 bits.
	GPIOC->ODR &= 0xFF00;
	//keep the bits that are the same as the data bits.
	GPIOC->ODR |= data;
	
	//call the delay function.
	delay(7000);
	
	//Reset enable, read, write and reset bits.
	GPIOB->BSRR = LCD_DM_DIS;
	
	//call the delay function.
	delay(7000);
}

void stringToLCD(char * word)
{
	bool nextLine = false;
	//Get the length of the string.
	uint16_t strLength = (strlen(word));
	
	if(strLength > 16)
		nextLine = true;
	
	for (int i = 0; i < strLength; i++)
	{
		if(nextLine == true && i == 16){
			//put the cursor on the second line
				commandToLCD(LCD_LN2);
		}
			
		//Call the data function with the pointer to message.
		dataToLCD(*word);
		//move to the next character in message.
		++word;
	}
}

void LCD_show(char * word){
	commandToLCD(LCD_CLR);
	commandToLCD(LCD_LN1);
	stringToLCD(word);
}