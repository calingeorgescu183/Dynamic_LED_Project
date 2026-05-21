#ifndef INC_LED_PWM_H_
#define INC_LED_PWM_H_


#include "main.h"

#define NUM_LEDS 4
extern volatile uint8_t global_brightness;
extern volatile uint8_t global_speed;

void LED_PWM_Init(void);
void LED_SetRGB(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b);
void LED_TurnOffAll(void);

#endif /* INC_LED_PWM_H_ */
