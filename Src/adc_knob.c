#include "adc_knob.h"
#include "led_pwm.h"
#include "diag_uart.h"


extern ADC_HandleTypeDef hadc1;

static uint32_t last_adc_tick = 0;
static uint32_t filtered_speed = 0; // Folosită pentru filtrul Low-Pass

void ADC_Knob_Update(void)
{

    if ((HAL_GetTick() - last_adc_tick) >= 50)
    {
        last_adc_tick = HAL_GetTick();

        ADC_ChannelConfTypeDef sConfig = {0};


        sConfig.Rank = ADC_REGULAR_RANK_1;

        sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;


        // 1. CITIRE LUMINOZITATE (IN5 - PA0)

        sConfig.Channel = ADC_CHANNEL_5;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        HAL_ADC_Start(&hadc1);

        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            uint32_t val = HAL_ADC_GetValue(&hadc1);


            if (val > 3900) val = 4095;

            // Liniar 0-100%
            uint32_t linear_pct = (val * 100) / 4095;


            if (override_bri == 0) {

                global_brightness = (uint8_t)((linear_pct * linear_pct) / 100);
            }
        }
        HAL_ADC_Stop(&hadc1);


        // 2. CITIRE VITEZĂ (IN6 - PA1)

        sConfig.Channel = ADC_CHANNEL_6;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        HAL_ADC_Start(&hadc1);

        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            uint32_t val = HAL_ADC_GetValue(&hadc1);
            if (val > 3900) val = 4095;

            uint32_t raw_speed = (val * 100) / 4095;


            if (raw_speed == 0) {
                raw_speed = 1;
            }

            // Filtru IIRpentru stabilitate "

            if (filtered_speed == 0) {
                filtered_speed = raw_speed * 10;
            } else {
                filtered_speed = (filtered_speed * 8 + (raw_speed * 10) * 2) / 10;
            }


            if (override_spd == 0) {
                global_speed = (uint8_t)(filtered_speed / 10);
            }
        }
        HAL_ADC_Stop(&hadc1);
    }
}
