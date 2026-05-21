#include "patterns.h"
#include "led_pwm.h"
#include <stdlib.h>

volatile PatternMode_t active_pattern = PATTERN_CHASER;
static PatternMode_t last_pattern = 255;

static uint32_t last_led_tick = 0;

// Variabile de stare pentru animații
static uint8_t step = 0;
static int16_t breathe_val = 0;
static int8_t breathe_dir = 1;
static uint8_t color_idx = 0;
static uint16_t wave_pos = 0; // Folosit pentru curgerea fluidă a valului

// Paletă de 6 culori pentru modul Breathe (R, G, B)
static const uint8_t breathe_palette[6][3] = {
    {255, 0, 0},     // Roșu
    {255, 200, 0},   // Galben/Portocaliu
    {0, 255, 0},     // Verde
    {0, 255, 255},   // Cyan
    {0, 0, 255},     // Albastru
    {255, 0, 255}    // Magenta
};

// Secvența pentru Ping-Pong (4 LED-uri)
static const uint8_t ping_pong_seq[] = {0, 1, 2, 3, 2, 1};

// Funcție utilitară pentru a stinge toate LED-urile instant
static void Clear_All_LEDs(void) {
    for(uint8_t i = 0; i < 4; i++) {
        LED_SetRGB(i, 0, 0, 0);
    }
}

void Patterns_Update(void)
{
    // 1. Detectăm dacă pattern-ul a fost schimbat
    if (active_pattern != last_pattern) {
        last_pattern = active_pattern;
        step = 0;
        breathe_val = 0;
        breathe_dir = 1;
        color_idx = 0;
        wave_pos = 0;
        Clear_All_LEDs();

        if (active_pattern == PATTERN_RANDOM) {
            srand(12345); // Seed determinist pentru reproductibilitate
        }
    }

    // 2. Calculăm Delay-ul (Viteza) pentru a respecta cerința de minim 50 pași/s
    uint32_t current_delay = 100;

    if (active_pattern == PATTERN_BREATHE) {
        current_delay = 15; // 66 pași/s
    }
    else if (active_pattern == PATTERN_WAVE) {
        // Wave-ul trebuie să fie ultra-fluid.
        // Generăm un delay curbat între 5ms (viteză extremă) și 45ms (lent, relaxant)
        uint32_t inv_speed = 100 - global_speed;
        current_delay = 5 + ((40 * inv_speed * inv_speed) / 10000);
    }
    else if (active_pattern == PATTERN_STROBO) {
        // Strobo limitat la 30ms pentru siguranță
        uint32_t inv_speed = 100 - global_speed;
        current_delay = 30 + ((150 * inv_speed * inv_speed) / 10000);
    }
    else {
        // Chaser, Ping-Pong, Random
        // Baza de 20ms garantează exact 50 pași/s la viteza maximă (1000ms / 20ms = 50 FPS)
        uint32_t inv_speed = 100 - global_speed;
        current_delay = 20 + ((1980 * inv_speed * inv_speed) / 10000);
    }

    // 3. Executăm animațiile (Non-Blocking)
    if (HAL_GetTick() - last_led_tick >= current_delay)
    {
        last_led_tick = HAL_GetTick();

        switch (active_pattern)
        {
            case PATTERN_CHASER:
                Clear_All_LEDs();
                LED_SetRGB(step, 0, 255, 255);            // Capul cometei
                LED_SetRGB((step + 3) % 4, 0, 80, 80);    // Coada 1
                LED_SetRGB((step + 2) % 4, 0, 10, 10);    // Coada 2

                step = (step + 1) % 4;
                break;

            case PATTERN_PING_PONG:
                Clear_All_LEDs();

                uint8_t r = (color_idx == 0) ? 255 : (color_idx == 3) ? 255 : 0;
                uint8_t g = (color_idx == 1) ? 255 : (color_idx == 3) ? 100 : 0;
                uint8_t b = (color_idx == 2) ? 255 : (color_idx == 4) ? 255 : 0;

                LED_SetRGB(ping_pong_seq[step], r, g, b);

                step = (step + 1) % 6;
                if (step == 0 || step == 3) {
                    color_idx = (color_idx + 1) % 5;
                }
                break;

            case PATTERN_STROBO:
                Clear_All_LEDs();
                if (step == 0 || step == 2) {
                    LED_SetRGB(0, 255, 0, 0);
                    LED_SetRGB(1, 255, 0, 0);
                }
                else if (step == 4 || step == 6) {
                    LED_SetRGB(2, 0, 0, 255);
                    LED_SetRGB(3, 0, 0, 255);
                }
                step = (step + 1) % 8;
                break;

            case PATTERN_BREATHE:
                {
                    int16_t inc = (global_speed / 7) + 1;

                    if (breathe_dir == 1) {
                        breathe_val += inc;
                        if (breathe_val >= 255) { breathe_val = 255; breathe_dir = -1; }
                    } else {
                        breathe_val -= inc;
                        if (breathe_val <= 0) {
                            breathe_val = 0;
                            breathe_dir = 1;
                            color_idx = (color_idx + 1) % 6;
                        }
                    }

                    uint8_t r_out = (breathe_palette[color_idx][0] * breathe_val) / 255;
                    uint8_t g_out = (breathe_palette[color_idx][1] * breathe_val) / 255;
                    uint8_t b_out = (breathe_palette[color_idx][2] * breathe_val) / 255;

                    for(uint8_t i = 0; i < 4; i++) {
                        LED_SetRGB(i, r_out, g_out, b_out);
                    }
                }
                break;

            case PATTERN_RANDOM:
                Clear_All_LEDs();
                uint8_t num_leds_to_light = (rand() % 2) + 1;

                for(uint8_t i = 0; i < num_leds_to_light; i++) {
                    uint8_t target_led = rand() % 4;

                    switch(rand() % 4) {
                        case 0: LED_SetRGB(target_led, 0, 255, 255); break;
                        case 1: LED_SetRGB(target_led, 255, 0, 255); break;
                        case 2: LED_SetRGB(target_led, 50, 50, 255); break;
                        case 3: LED_SetRGB(target_led, 150, 255, 0); break;
                    }
                }
                break;

            case PATTERN_WAVE:
                // Trecere fluidă: R -> G -> B -> W -> R...
                // Folosim o roată a culorilor de 256 pași, împărțită în 4 sferturi de 64 pași.
                for (uint8_t i = 0; i < 4; i++) {
                    // Decalăm poziția pe cerc pentru fiecare LED (creează efectul de "șarpe")
                    // Distanța de 45 de unități asigură acoperirea culorilor simultane
                    uint8_t pos = (wave_pos + (i * 45)) % 256;

                    // Calculăm tranziția liniară între două culori
                    uint8_t val = ((pos % 64) * 255) / 63;
                    uint8_t inv = 255 - val;

                    uint8_t r = 0, g = 0, b = 0;

                    switch(pos / 64) {
                        case 0: // Roșu către Verde
                            r = inv; g = val; b = 0;
                            break;
                        case 1: // Verde către Albastru
                            r = 0;   g = inv; b = val;
                            break;
                        case 2: // Albastru către Alb (R și G urcă împreună la 255)
                            r = val; g = val; b = 255;
                            break;
                        case 3: // Alb către Roșu (G și B scad, R rămâne 255)
                            r = 255; g = inv; b = inv;
                            break;
                    }

                    LED_SetRGB(i, r, g, b);
                }

                // Avansăm valul continuu (la viteză mică curge încet, la viteză mare fuge)
                wave_pos = (wave_pos + 1) % 256;
                break;
        }
    }
}
