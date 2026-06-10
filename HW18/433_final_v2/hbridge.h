#ifndef HBRIDGE_H
#define HBRIDGE_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define IN1 12
#define IN2 13

void init_hbridge();
void set_duty(float duty);
#endif
