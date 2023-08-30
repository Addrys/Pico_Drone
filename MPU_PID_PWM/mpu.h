#ifndef MPU_H
#define MPU_H
#endif

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>
#include <time.h>

#ifndef _CLOCKS_PER_SEC_
#define _CLOCKS_PER_SEC_ 1000
#endif



static void mpu6050_reset();

void mostrar_offsets();
void set_offsets(int16_t gx_o, int16_t gy_o, int16_t gz_o, int16_t ax_o, int16_t ay_o, int16_t az_o);
static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) ;
void mpu6050_run();