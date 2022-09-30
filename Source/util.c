 /*
File: util.c
Purpose: All of the initialization functions for the pins/clocks

Author: Erika McCluskey, Hayden Jin
*/
 
#include "util.h"
#include "stm32f10x.h"

void delay(uint32_t delay)
{
  uint32_t i=0;
	for(i=0; i< delay; ++i)
	{
	}
}

void led_IO_init (void)
{
    //Enable peripheral clocks for various ports and subsystems
    //Port A 
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
		// PA6 -> 0011 General Purpose output push-pull 50MHz
		// Red LED
		GPIOA->CRL |= GPIO_CRL_MODE6;
    GPIOA->CRL &= ~GPIO_CRL_CNF6;
		
	  // PA7 -> 0011 General Purpose output push-pull 50MHz
		// Green LED
		GPIOA->CRL |= GPIO_CRL_MODE7;
    GPIOA->CRL &= ~GPIO_CRL_CNF7;	
}	
	
void button_IO_init(void){
		//Enable peripheral clocks for various ports and subsystems
    //Port A and Port B
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN;
		
		// PB10 -> 0100 Floating input input mode
		// Blue button (add fingerprint)
		GPIOB->CRH |= GPIO_CRH_CNF10_0;
		GPIOB->CRH &= ~GPIO_CRH_CNF10_1 & ~GPIO_CRH_MODE10;
		
		// PB9 -> 0100 Floating input input mode
		// Green button (verify fingerprint)
		GPIOB->CRH |= GPIO_CRH_CNF9_0;
		GPIOB->CRH &= ~GPIO_CRH_CNF9_1 & ~GPIO_CRH_MODE9;
		
		// PA5 -> 0100 Floating input input mode 
		// Red button (reset all)
		GPIOA->CRL |= GPIO_CRL_CNF5_0;
		GPIOA->CRL &= ~GPIO_CRL_CNF5_1 & ~GPIO_CRL_MODE5;
		
}

void buzzer_IO_init(void)
{
	//Enable peripheral clocks for various ports and subsystems
   //Port B and AFIO and TIM3
	RCC->APB2ENR |=  RCC_APB2ENR_IOPBEN & RCC_APB2ENR_AFIOEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	
	// PB8 as AF output push-pull 50MHz
	GPIOB->CRH |= GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0 ;
  GPIOB->CRH &= ~GPIO_CRH_CNF8_0;

	tim3_init();
}

void tim3_init(void){
	TIM3->CR1 |= TIM_CR1_CEN; // Enable Timer3
	TIM3->EGR |= TIM_EGR_UG; // Reinitialize the counter
	TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // PWM mode 1
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC1FE; // Preload Enable, Fast Enable
	TIM3->CCER |= TIM_CCER_CC1E; // Enable CH1
	TIM3->PSC = 0x095F; // Divide 24 MHz by 2400 (PSC+1), PSC_CLK= 10000 Hz, 1 count = 0.1 ms
	TIM3->ARR = 100; // 100 counts = 10 ms or 100 Hz
	TIM3->CCR1 = 0; // 50 counts = 5 ms = 50% duty cycle
	TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; // Enable Timer3
}

void reset_buzzer(void)
{
		//disable TIM3
		TIM3->CR1 &= ~TIM_CR1_CEN; 
		
		// reset PB8 to reset state 
		GPIOB->CRH |= GPIO_CRH_CNF8_0;
		GPIOB->CRH &= ~GPIO_CRH_CNF8_1 & ~GPIO_CRH_MODE8;
}

void soundSensor_init(void)
{
	// Enable clocks
	RCC->APB2ENR |=  RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_ADC1EN;
	
	//Configure as PA4 as Analog input (Sound sensor)
	GPIOA->CRL &= ~GPIO_CRL_MODE4;
	GPIOA->CRL &= ~GPIO_CRL_CNF4;
	
	//Turn on ADC
	ADC1->CR2 |= 0x00000001;
}
	
	
void fingerprint_sensor_pin_init(void){
	
	RCC->APB2ENR |=  RCC_APB2ENR_IOPAEN |  RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	
	//PA5 - (General Purpose Push-Pull 50MHz)
		GPIOA->CRL |= GPIO_CRL_MODE5;
    GPIOA->CRL &= ~GPIO_CRL_CNF5;
	
	//PB5- RST pin (General Purpose Push-Pull 50MHz)
		GPIOB->CRL |= GPIO_CRL_MODE5;
    GPIOB->CRL &= ~GPIO_CRL_CNF5;
	
	//PB3 - Wake pin (Floating input)
		GPIOB->CRL |= GPIO_CRL_CNF3_0;
		GPIOB->CRL &= ~GPIO_CRL_CNF3_1 & ~GPIO_CRL_MODE3;
	
	//PA9 - Tx (General Purpose Push-Pull 50MHz)
		GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0 ;
    GPIOA->CRH &= ~GPIO_CRH_CNF9_0;

	//PA10 - RX (Floating input)
		GPIOA->CRH |= GPIO_CRH_CNF10_1;
		GPIOA->CRH &= ~GPIO_CRH_CNF10_0 & ~GPIO_CRH_MODE10;
	
}

void dma_interrupt_init(void){
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	//set priority and enable channel 4 for usart1
	DMA1_Channel4->CCR |= DMA_CCR4_TEIE | DMA_CCR4_HTIE | DMA_CCR4_TCIE | DMA_CCR4_MINC ;
	DMA1_Channel4->CCR &= ~DMA_CCR4_DIR & ~DMA_CCR4_PSIZE & ~DMA_CCR4_MSIZE & ~DMA_CCR4_PL & ~DMA_CCR4_CIRC;
	DMA1_Channel4->CCR |= DMA_CCR4_EN;
	
	//set priority and enable channel 5 for usart1
	DMA1_Channel5->CCR |= DMA_CCR5_TEIE | DMA_CCR5_HTIE | DMA_CCR5_TCIE | DMA_CCR5_MINC ;
	DMA1_Channel5->CCR &= ~DMA_CCR5_DIR & ~DMA_CCR5_PSIZE & ~DMA_CCR5_MSIZE & ~DMA_CCR5_PL & ~DMA_CCR5_CIRC;
	DMA1_Channel5->CCR |= DMA_CCR5_EN;
}


