#include "util.h"
#include "testCode.h"
#include "stm32f10x.h"

void test_LEDs(void){

  //NOTE : MUST COMMENT OUT ONE OF THE PA6 BUTTONS TO TEST 
	
	//button PA5 (Red) should turn on PA6 (Red LED)
	if ((GPIOA->IDR & GPIO_IDR_IDR5) == 0)
  {
     GPIOA->BSRR = GPIO_BSRR_BS6;
  }
  else
  {
     GPIOA->BSRR = GPIO_BSRR_BR6;
  }
	
	/*
	//button PB10 (Blue) should turn on PA7 (Green LED)
	if ((GPIOB->IDR & GPIO_IDR_IDR10) == 0)
  {
     GPIOA->BSRR = GPIO_BSRR_BS6;
  }
  else
  {
     GPIOA->BSRR = GPIO_BSRR_BR6;
  }
	*/
	
	//button PB9 (Green) should turn on PA7 (Green LED)
	if ((GPIOB->IDR & GPIO_IDR_IDR9) == 0)
  {
     GPIOA->BSRR = GPIO_BSRR_BS7;
  }
  else
  {
     GPIOA->BSRR = GPIO_BSRR_BR7;
  }
}

void test_buzzer(void)
{
		delay(6000000);
		reset_buzzer();
		delay(6000000);
		buzzer_IO_init();
}

void test_soundSensor(void)
 {
		uint32_t data;
		data = readSensorData();
		
		if (data > 2900)
		{
			GPIOA->BSRR = GPIO_BSRR_BS6;
		}
		else
		{
			GPIOA->BSRR = GPIO_BSRR_BR6;
		}
}

