 /*
File: main.c
Purpose: Main program file for the system

Author: Erika McCluskey, Hayden Jin
*/

#include "STMicroelectrionicsStartup.h"
#include "util.h"
#include "lcd.h"
#include "fingerprint.h"
#include "commands.h"

int main(void)
{
	//configure clock
		SystemClock_Config();
	//initialize fingerprint sensor GPIO pins
		fingerprint_sensor_pin_init();
	//initializate DMA channels for interrupts
		dma_interrupt_init();
	//initialize TIM3
		tim3_init();
	//initialize usart1
		usart1_setup();	
	//initialized LED GPIO pins
  	led_IO_init();
	//initlizalize button GPIO pins
	  button_IO_init();
	//initlizalize buzzer PWM
  	buzzer_IO_init();
	//turn buzzer off
		reset_buzzer();
	//initialize sound sensor GPIO pins and ADC
	  soundSensor_init();
	//intialize LCD GPIO pins and displau
		lcd_init();
	//display Welcome message
		LCD_show("Welcome! Awaiting input...");
	//set compare level of fingerprint sensor
		device_startup();
	//notify user that device is ready for use 
	  device_ready();
	
		//system functionality
  while (1)
  {		
		uint32_t i=0;
		uint32_t result;
		uint32_t data;
		char action;
			
		//give the user time to make a selection
		for(i=0; i < 60000000; ++i)
		{
			action = state_of_buttons();
			if(action != '0')
				{	// 0 means none of the buttons have been pressed, keep looping	
					break;
				}
		}

		switch(action)
		{
			//add user	
			case '1':
				  LCD_show("Place finger on sensor...");
					for(i=0; i < 10; ++i) //give the user time to place their finger
					{
						result = AddUser();
						if(result == ACK_SUCCESS){	
							LCD_show("Success! User added.");
							flash_LED('G', 5, 3000000);		
							break;
						}
					}
					if(result != ACK_SUCCESS){
						LCD_show("Please try again...");
						flash_LED('R', 2, 600000);		
					}	
					break;
			
			//verify user
			case '2':
				LCD_show("Awaiting to verify...");
				//give user time to make sound
				for(i=0; i < 600000; ++i) //give the user time to make a sound
				{	
					data = readSensorData(); //sound level
					if(data > 2900)
					{	// greater than 2900 means a relatively loud noise (i.e. speaking loudly) 
						// loop 5 times to give user time to add their finger to the sensor for verification
						for(i=0; i < 5; ++i)
						{
							result = VerifyUser();
							if(result == ACK_SUCCESS)
							{
								LCD_show("Success! You're in!");
								authorized();				
								break;
							}
						}	
						if(result != ACK_SUCCESS)
						{
							LCD_show("Sorry, You're not authorized.");
							flash_LED('R', 5, 3000000);				
						}
						break;
					}	
				}
				
				if (data < 2900)
				{
					LCD_show("LOUDER!");
					flash_LED('R', 5, 3000000);
				}

				break;
				
			//reset the system by deleting all fingerprints
			case '3':
					ClearAllUser();
					LCD_show("System reset.");
					flash_LED('G', 10, 300000);	
					break;
			
			default: break;
			}		
		}
}

