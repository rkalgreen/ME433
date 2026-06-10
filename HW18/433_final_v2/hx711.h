#include <stdio.h>
#include "pico/stdlib.h"

// HX711 pin definitions
#define DT_PIN 17
#define SCK_PIN 16



void HX711_init();
int32_t HX711_read();