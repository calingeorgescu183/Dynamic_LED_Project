#include "led_pwm.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim15;
volatile uint8_t global_brightness = 50;
volatile uint8_t global_speed = 50;
const uint8_t gamma8[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 26, 27,
    29, 31, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53, 55, 57, 60,
    62, 65, 68, 70, 73, 76, 79, 82, 85, 88, 91, 94, 98, 101, 104, 108,
    112, 115, 119, 123, 127, 131, 135, 139, 143, 147, 152, 156, 161, 165, 170, 175,
    179, 184, 189, 194, 199, 204, 209, 215, 220, 225, 231, 236, 242, 248, 253, 255
};

void LED_PWM_Init(void)
{
    /* LED1: TIM1 (R -> CH1, B -> CH2, G -> CH3) */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    /* LED2: TIM2 (R -> CH1, B -> CH2, G -> CH3) */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    /* LED3: TIM3 (R -> CH1, B -> CH2, G -> CH3) */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

    /* LED4: TIM4 (R -> CH1, B -> CH2, G -> CH3) */
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);

    LED_TurnOffAll();
}

/**

 led_index: Indexul LED-ului (0 pentru LED1, 1 pentru LED2, etc.)
 r: Intensitatea pe canalul Roșu (0 - 255)
 g: Intensitatea pe canalul Verde (0 - 255)
 b: Intensitatea pe canalul Albastru (0 - 255)
 */
void LED_SetRGB(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b)
{

	    uint8_t gamma_r = gamma8[r / 2];
	    uint8_t gamma_g = gamma8[g / 2];
	    uint8_t gamma_b = gamma8[b / 2];

	    // 2. gamma correction
	    uint16_t final_r = (gamma_r * global_brightness) / 100;
	    uint16_t final_g = (gamma_g * global_brightness) / 100;
	    uint16_t final_b = (gamma_b * global_brightness) / 100;
	switch(led_index)
	    {
	        case 0: /* LED1 */
	            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, final_r); /* PA8  - R */
	            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, final_b); /* PA9  - B */
	            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, final_g); /* PA10 - G */
	            break;

	        case 1: /* LED2 */
	            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, final_r); /* PA5  - R */
	            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, final_b); /* PB3  - B */
	            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, final_g); /* PA11 - G */
	            break;

	        case 2: /* LED3 */
	            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, final_r); /* PA6  - R */
	            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, final_b); /* PA7  - B */
	            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, final_g); /* PB9  - G */
	            break;

	        case 3: /* LED4 */
	            __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, final_r); /* PC6  - R */
	            __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, final_b); /* PC7  - B */
	            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, final_g); /* PB8  - G */
	            break;

	        default:
	            /* Index invalid, ignore */
	            break;
	    }
}


void LED_TurnOffAll(void)
{
    for(uint8_t i = 0; i < NUM_LEDS; i++)
    {
        LED_SetRGB(i, 0, 0, 0);
    }
}
