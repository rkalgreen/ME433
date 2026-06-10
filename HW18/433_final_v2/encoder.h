#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define AS5600_ADDR 0x36
#define RAW_ANGLE 0x0C
#define ANGLE 0x0E
#define ZPOS 0x01
#define MPOS 0x03
#define MAGNET 0x0B

void encoder_init();
int read_angle();
void read_data(uint8_t addr, uint8_t* buf);