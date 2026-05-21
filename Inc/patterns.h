#ifndef PATTERNS_H
#define PATTERNS_H

#include "main.h"

typedef enum {
    PATTERN_CHASER = 0,
    PATTERN_BREATHE,
    PATTERN_PING_PONG,
    PATTERN_STROBO,
    PATTERN_RANDOM,
	PATTERN_WAVE
} PatternMode_t;

extern volatile PatternMode_t active_pattern;

void Patterns_Update(void);

#endif
