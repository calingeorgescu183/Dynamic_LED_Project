#ifndef UART_CLI_H
#define UART_CLI_H

#include "main.h"

// Steaguri globale pentru override și moduri
extern uint8_t override_spd;
extern uint8_t override_bri;
extern uint8_t demo_mode;

void CLI_Init(void);
void CLI_Process(void);

#endif
