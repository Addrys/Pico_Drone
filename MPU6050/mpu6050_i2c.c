/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>
#include <time.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define PULSE_PIN 1
volatile uint32_t last_pulse_time = 0;



#ifndef _CLOCKS_PER_SEC_
#define _CLOCKS_PER_SEC_ 1000
#endif
/* Example code to talk to a MPU6050 MEMS accelerometer and gyroscope

   This is taking to simple approach of simply reading registers. It's perfectly
   possible to link up an interrupt line and set things up to read from the
   inbuilt FIFO to make it more useful.

   NOTE: Ensure the device is capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefor I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO PICO_DEFAULT_I2C_SDA_PIN (On Pico this is GP4 (pin 6)) -> SDA on MPU6050 board
   GPIO PICO_DEFAULT_I2C_SCL_PIN (On Pico this is GP5 (pin 7)) -> SCL on MPU6050 board
   3.3v (pin 36) -> VCC on MPU6050 board
   GND (pin 38)  -> GND on MPU6050 board
*/

// By default these devices  are on bus address 0x68
static int addr = 0x68;
int write_block,read_block = 0;
#ifdef i2c_default
static void mpu6050_reset() {
    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    uint8_t buf[] = {0x6B, 0x00}; //cambio el 0x80 por 0x00 para quitar el modo sleep de la mpu
    i2c_write_blocking(i2c_default, addr, buf, 2, false);
}

void mostrar_offsets(){
    //Printeamos los offsets que tiene configurados la MPU6050
    uint8_t buffer[6]; 
    //OBTENEMOS LOS OFFSETS DEL GIROSCOPIO
    int16_t gx_o,gy_o,gz_o,ax_o,ay_o,az_o;

    uint8_t val = 0x13; //dir de offsetX
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true);
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    gx_o = (int16_t)((buffer[0]<<8)  |buffer[1]);
   
    val = 0x14; //dir de offset Y
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true);
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    gy_o = (int16_t)((buffer[0]<<8)  |buffer[1]);

    val = 0x15; //dir de offset Z
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true); 
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    gz_o = (int16_t)((buffer[0]<<8)  |buffer[1]);
    
    //OBTENEMOS LOS OFFSETS DEL ACELEROMETRO
    val = 0x06; //dir de offsetX
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true);
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    ax_o = (int16_t)((buffer[0]<<8)  |buffer[1]);

    val = 0x08; //dir de offset Y
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true);
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    ay_o = (int16_t)((buffer[0]<<8)  |buffer[1]);

    val = 0x0A; //dir de offset Z
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true); 
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    az_o = (int16_t)((buffer[0]<<8)  |buffer[1]);
    /*
    printf("Printeamos AZ_o\n");
    for(int i = 0 ; i < 6; i++)
    {
        printf("\n%d",buffer[i]);
    }
    */
    printf("Offsets Antes\n[gyrXYZ], X:%d, Y:%d, Z:%d",gx_o,gy_o,gz_o);
    printf("\n[accXYZ], X:%d, Y:%d, Z:%d\n",ax_o,ay_o,az_o);
}

void set_offsets(int16_t gx_o, int16_t gy_o, int16_t gz_o, int16_t ax_o, int16_t ay_o, int16_t az_o){
    uint8_t buf[3];

    buf[0] = 0x13;
    buf[1] = (uint8_t)(gx_o >> 8);
    buf[2] = (uint8_t)(gx_o & 0xFF);
    i2c_write_blocking(i2c_default, addr, buf, 3, false);

    buf[0] = 0x15;
    buf[1] = (uint8_t)(gy_o >> 8);
    buf[2] = (uint8_t)(gy_o & 0xFF);
    i2c_write_blocking(i2c_default, addr, buf, 3, false);

    buf[0] = 0x17;
    buf[1] = (uint8_t)(gz_o >> 8);
    buf[2] = (uint8_t)(gz_o & 0xFF);
    i2c_write_blocking(i2c_default, addr, buf, 3, false);

    buf[0] = 0x06;
    buf[1] = (uint8_t)(ax_o >> 8);
    buf[2] = (uint8_t)(ax_o & 0xFF);
    i2c_write_blocking(i2c_default, addr, buf, 3, false);

    buf[0] = 0x08;
    buf[1] = (uint8_t)(ay_o >> 8);
    buf[2] = (uint8_t)(ay_o & 0xFF);
    i2c_write_blocking(i2c_default, addr, buf, 3, false);

    buf[0] = 0x0A;
    buf[1] = (uint8_t)(az_o >> 8);
    buf[2] = (uint8_t)(az_o & 0xFF);
    i2c_write_blocking(i2c_default, addr, buf, 3, false);
}

/**
 * Esta funcion sirve para limpiar el bit de la interrupcion, es decir para decirle que la hemos recibido
*/
static void Read_INT(){
    uint8_t buffer[1];
    uint8_t val = 0x3A;
    
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true); // true to keep master control of bus
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 1, false);
    //printf(" Valor del registro INT_STATUS : %u\n", buffer[0]);
}
static void Read_INT_Enable(){
    uint8_t buffer[1];
    uint8_t val = 0x38;
    
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true); // true to keep master control of bus
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 1, false);
    printf(" Valor del registro INT_ENABLE : %u\n", buffer[0]);
}
static void Write_INT_Enable(){
    uint8_t buf[] = {0x38, 0x01}; //0x38 es la direccion del registro y 0x01 el valor que quiero poner
    i2c_write_blocking(i2c_default, addr, buf, 2, false);
    printf("Escribimos %u en INT Enable",buf[1]);
}
static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    uint8_t buffer[6];
    
    // Start reading acceleration registers from register 0x3B for 6 bytes
    uint8_t val = 0x3B;
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true); // true to keep master control of bus
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);
    /*
    printf ("\n");
    printf("w/b: ");
    printf("%d",write_block); 
    printf("\t");
    printf("%d",read_block);
    printf("\n");
    //--------------------------------------
     for (int i = 0; i < 6; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
    //-------------------------------*/
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
    
    // Now gyro data from reg 0x43 for 6 bytes
    // The register is auto incrementing on each read
    val = 0x43;
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true);
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 6, false);  // False - finished with bus
        //--------------------------------------
        /*
            printf ("\n");
    printf("w/b: ");
    printf("%d",write_block); 
    printf("\t");
    printf("%d",read_block);
    printf("\n");
     for (int i = 0; i < 6; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
    //-------------------------------*/
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);;
    }

    // Now temperature from reg 0x41 for 2 bytes
    // The register is auto incrementing on each read

    val = 0x41;
    write_block = i2c_write_blocking(i2c_default, addr, &val, 1, true);
    read_block = i2c_read_blocking(i2c_default, addr, buffer, 2, false);  // False - finished with bus
    /*printf ("\n");
    printf("w/b: ");
    printf("%d",write_block); 
    printf("\t");
    printf("%d",read_block);
    printf("\n");
            //--------------------------------------
     for (int i = 0; i < 6; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");*/
    //-------------------------------
    *temp = buffer[0] << 8 | buffer[1];
}
#endif

void pulse_handler(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    uint32_t time_since_last_pulse = current_time - last_pulse_time;
    last_pulse_time = current_time;
    printf("Tiempo desde el último pulso: %u microsegundos\n", time_since_last_pulse);
    Read_INT(); //Limpiamos la el flag de interrupcion
    
}

int main() {
    stdio_init_all();

    sleep_ms(4000);
    //gpio_init(15);
    //gpio_set_dir(15, GPIO_OUT);
    //gpio_init(16);
    //gpio_set_dir(16, GPIO_OUT);
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/mpu6050_i2c example requires a board with I2C pins
    puts("Default I2C pins were not defined");
    //gpio_put(15, 1);
#else
    //gpio_put(16, 1); //derecha, funciona correctamente.
    printf("Hello, MPU6050! Reading raw data from registers...\n");

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    //i2c_init(i2c_default, 400 * 1000);
    i2c_init(i2c_default, 100000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    mpu6050_reset();
    sleep_ms(1000);
    Read_INT_Enable();
    Write_INT_Enable();
    sleep_ms(1000);
    Read_INT_Enable();

    Read_INT();

/*
    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_IN);
    
    // Configuración del timer para generar interrup

    // Configuración de la interrupción para el pin de pulso
    gpio_set_irq_enabled_with_callback(PULSE_PIN, GPIO_IRQ_EDGE_FALL, true, &pulse_handler);

    while (1) {
        tight_loop_contents();
    }*/


    int16_t acceleration[3], gyro[3], temp;
    //float en vez de double por ahorro de memoria y mejor rendimiento, pensando en la RP2040
    //clock_t tiempo_previo_ticks = clock(); //está en ciclos del reloj, hay que pasarlo a milisegundos
    
    uint32_t tiempo_previo, tiempo_actual; //esta en microsegundos( 10 ^-6)
    float dt = 0.0;
    float ang_x , ang_y = 0.0;              //--INICIALIZO TODAS LAS VARIABLES A 0
    float ang_x_prev, ang_y_prev = 0.0;
    int ax, ay, az = 0;
    int gx, gy = 0;
    
    
    float accel_ang_x, accel_ang_y = 0.0; //inicializamos a 0
    //double timepo_previo = (double)tiempo_previo_ticks * 1000 / CLOCKS_PER_SEC;
    
    mostrar_offsets(); //funcion propia para mostrar los offsetss
    set_offsets(-303,86,-38,-1493,1868,1243);  //funcion propia para poner los offsets calculados con el arduino
    
    //Añadimos el manejador para el tiempo
    
    tiempo_previo = time_us_32();
    while (1) {
        
        mpu6050_read_raw(acceleration, gyro, &temp);
        //mostrar_offsets();
        ax = acceleration[0]; ay = acceleration[1]; az = acceleration[2];
        gx = gyro[0]; gy = gyro[1]; 
        
        tiempo_actual = time_us_32();
        dt = (float)(tiempo_actual - tiempo_previo)/1000000;
        tiempo_previo = tiempo_actual; //Actualizamos el tiempo previo tras el calculo de dT

        //Calculamos los ángulos con el acelerometro
        accel_ang_x = 0.0;
        if(sqrt(pow(ax,2) + pow(az,2)) != 0){   //compruebo division por 0
            accel_ang_x = atan(ay/sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
        }
        accel_ang_y = 0.0;
        if(sqrt(pow(ay,2) + pow(az,2)) != 0){
            accel_ang_y=atan(-ax/sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
        }
        
        
        //Calculamos angulo de rotacion con giroscopio y filtro complemento
        ang_x = 0.98 * (ang_x_prev + (gx/131) * dt ) + 0.02 * accel_ang_x;
        ang_y = 0.98 * (ang_y_prev + (gy/131) * dt ) + 0.02 * accel_ang_y;
        
        ang_x_prev=ang_x;
        ang_y_prev=ang_y;
        //SALIDAS TIPO PARA DEBUG
        /*
        printf("\n DT = %f, Tactual = %u",dt,tiempo_actual);
        printf("%d,%d,%d,%d,%d\n",ax,ay,az,gx,gy);
        printf("Rotacion en Xa: \t %f",accel_ang_x);
        printf("\tRotacion en Ya: \t %f\n",accel_ang_y);
        printf("Rotacion en X: \t %f",ang_x);
        printf("\tRotacion en Y: \t %f\n",ang_y);
        */
        //SALIDAS TIPO PARA PROGRAMA
        //printf("%d,%d,%d,%d,%d,%d", acceleration[0], acceleration[1], acceleration[2], gyro[0], gyro[1], gyro[2]); //programa original

        //printf("%f,%f\n",ang_x,ang_y);
       
        //sleep_ms(1); //Lo ideal seria esperar a la interrupcion de la MPU
        uint32_t current_time = time_us_32();
        uint32_t time_since_last_pulse = current_time - last_pulse_time;
        last_pulse_time = current_time;
        printf("Tiempo desde el último pulso: %u microsegundos\n", time_since_last_pulse);
    }


#endif
    return 0;
}