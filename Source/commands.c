 /*
File: commands.c 
Purpose: Functions to operate the system to make the main.c more readable

Author: Erika McCluskey, Hayden Jin
*/

#include "commands.h"
#include "stm32f10x.h"

void turn_on_green_LED(void){
	GPIOA->BSRR = GPIO_BSRR_BS7;
}

void turn_off_green_LED(void){
	GPIOA->BSRR = GPIO_BSRR_BR7;
}

void turn_on_red_LED(void){
	GPIOA->BSRR = GPIO_BSRR_BS6;
}

void turn_off_red_LED(void){
	GPIOA->BSRR = GPIO_BSRR_BR6;
}

void flash_LED(char colour, int howManyTimes, int delayTime)
{
		while (howManyTimes > 0) 
		{
			if(colour == 'G')
				turn_on_green_LED();
			else
				turn_on_red_LED();
				
			delay(delayTime);
			
			if(colour == 'G')
				turn_off_green_LED();
			else
				turn_off_red_LED();
				
			delay(delayTime);
				
			howManyTimes--;
		}
}

bool add_button_pressed(void){
	//PB10 -> blue button
	if ((GPIOB->IDR & GPIO_IDR_IDR10) == 0){
		return true;
	}
	else
		return false;
}

bool verify_button_pressed(void){
	//PB9 -> green button
	if ((GPIOB->IDR & GPIO_IDR_IDR9) == 0){
		return true;
	}
	else
		return false;
}

bool reset_button_pressed(void){
	//PA5 -> red button
	if ((GPIOA->IDR & GPIO_IDR_IDR5) == 0){
		return true;
	}
	else
		return false;
}

char state_of_buttons(void){
	if(add_button_pressed())
		return '1';
	else if(verify_button_pressed())
		return '2';
	else if(reset_button_pressed())
		return '3';
	else
		return '0';
}

void authorized(void){
		flash_LED('G', 2, 3000000);
		reset_buzzer();
		delay(6000000);
		buzzer_IO_init();
		delay(6000000);
		flash_LED('G', 2, 3000000);
		reset_buzzer();
		delay(6000000);
		buzzer_IO_init();
		delay(6000000);
		reset_buzzer();
		flash_LED('G', 2, 3000000);
}

void device_ready(void){
	flash_LED('G', 2, 3000000);
}

uint32_t readSensorData(void)
{
	uint32_t data = 0;

	ADC1->SQR3 |= 0x00000004;
	
	ADC1->CR2 |= 0x00000001;
	
	while((ADC1->SR & 0x00000002) != 0x00000002);

	data = ADC1->DR;
	
	return data;
}


