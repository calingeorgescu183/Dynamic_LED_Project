#include "diag_uart.h"
#include "patterns.h" // Pentru a vedea active_pattern
#include "led_pwm.h"  // Pentru a vedea global_speed și global_brightness
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2; // Handler-ul generat de CubeMX

uint8_t override_spd = 0;
uint8_t override_bri = 0;
uint8_t demo_mode = 0; // 0 = MANUAL, 1 = DEMO

#define RX_BUF_SIZE 32
static uint8_t rx_byte;
static char rx_buffer[RX_BUF_SIZE];
static uint8_t rx_index = 0;
static uint8_t cmd_ready = 0;

// Inițializăm ascultarea pe UART
void CLI_Init(void) {
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

    char *msg = "\r\n=== Sistem Initializat ===\r\nTastati comenzi (PAT, SPD, BRI, STAT, DEMO, MANUAL):\r\n> ";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}

// Această funcție este apelată automat de HAL de fiecare dată când vine o literă nouă
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        if (rx_byte == '\n' || rx_byte == '\r') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0'; // Terminăm string-ul
                cmd_ready = 1;
            }
        } else {
            if (rx_index < RX_BUF_SIZE - 1) {
                rx_buffer[rx_index++] = rx_byte;
            }
        }
        // Repornim ascultarea pentru următoarea literă
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

// Funcția utilitară pentru a trimite mesaje înapoi pe PC
static void CLI_Print(char* str) {
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

// Logica de procesare a comenzilor (Rulată în main)
void CLI_Process(void) {
    if (cmd_ready) {
        char response[64];
        int val;

        // 1. Verificăm MAI ÎNTÂI comenzile textuale complete
        if (strcmp(rx_buffer, "SPD AUTO") == 0) {
            override_spd = 0;
            sprintf(response, "Viteza redata potentiometrului.\r\n");
        }
        else if (strcmp(rx_buffer, "BRI AUTO") == 0) {
            override_bri = 0;
            sprintf(response, "Brightness redat potentiometrului.\r\n");
        }
        else if (strcmp(rx_buffer, "DEMO") == 0) {
            demo_mode = 1;
            sprintf(response, "Mod DEMO activat. Auto-ciclare 5s.\r\n");
        }
        else if (strcmp(rx_buffer, "MANUAL") == 0) {
            demo_mode = 0;
            sprintf(response, "Mod MANUAL activat.\r\n");
        }
        else if (strcmp(rx_buffer, "STAT") == 0) {
            sprintf(response, "--- STATUS ---\r\nPattern: %d\r\nSpeed: %d (Ovr: %d)\r\nBri: %d (Ovr: %d)\r\nMod: %s\r\n",
                    active_pattern, global_speed, override_spd, global_brightness, override_bri, demo_mode ? "DEMO" : "MANUAL");
        }
        // 2. Abia apoi verificăm comenzile cu parametri numerici
        else if (strncmp(rx_buffer, "PAT ", 4) == 0) {
            if (sscanf(rx_buffer + 4, "%d", &val) == 1 && val >= 0 && val <= PATTERN_WAVE) {
                active_pattern = (PatternMode_t)val;
                sprintf(response, "Pattern schimbat la: %d\r\n", val);
            } else {
                sprintf(response, "Eroare: Pattern invalid (0-5).\r\n");
            }
        }
        else if (strncmp(rx_buffer, "SPD ", 4) == 0) {
            if (sscanf(rx_buffer + 4, "%d", &val) == 1 && val >= 0 && val <= 100) {
                global_speed = val;
                override_spd = 1; // Blocăm citirea din ADC
                sprintf(response, "Viteza fortata la: %d%%\r\n", val);
            } else {
                sprintf(response, "Eroare: Viteza trebuie sa fie 0-100.\r\n");
            }
        }
        else if (strncmp(rx_buffer, "BRI ", 4) == 0) {
            if (sscanf(rx_buffer + 4, "%d", &val) == 1 && val >= 0 && val <= 100) {
                global_brightness = (uint8_t)((val * val) / 100);
                override_bri = 1; // Blocăm citirea din ADC
                sprintf(response, "Brightness fortat la: %d%%\r\n", val);
            } else {
                sprintf(response, "Eroare: Brightness trebuie sa fie 0-100.\r\n");
            }
        }
        else {
            sprintf(response, "Comanda necunoscuta.\r\n");
        }

        CLI_Print(response);
        CLI_Print("> ");

        // Curățăm buffer-ul pentru comanda următoare
        rx_index = 0;
        cmd_ready = 0;
    }
}
