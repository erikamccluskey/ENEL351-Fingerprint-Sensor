 /*
File: util.h
Purpose: Header file for util.c
*/

#include <stdint.h>
#include <stdbool.h>

void delay(uint32_t delay);
void led_IO_init (void);
void button_IO_init(void);
void buzzer_IO_init(void);
void reset_buzzer(void);
void soundSensor_init(void);
void tim3_init();
void ePulse (void);
void fingerprint_sensor_pin_init(void);
void dma_interrupt_init(void);







