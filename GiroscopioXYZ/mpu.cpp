#include "mpu.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>
#include <time.h>
#include "pid.h"

static int addr = 0x68;
int write_block,read_block = 0;

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


void mpu6050_run(){
    //Ajustamos los coeficientes del controlador PID
    float kp = 2.0;
    float ki = 0.1;
    float kd = 1.0;
    //creamos los controladores
    controladorPID pidMotor1_roll(kp,ki,kd,true);
    controladorPID pidMotor2_roll(kp,ki,kd,true);//Roll + para M2
    controladorPID pidMotor3_roll(kp,ki,kd,false);
    controladorPID pidMotor4_roll(kp,ki,kd,false);
    controladorPID pidMotor1_pitch(kp,ki,kd,true);
    controladorPID pidMotor2_pitch(kp,ki,kd,false);//Pitch - para M2
    controladorPID pidMotor3_pitch(kp,ki,kd,true);
    controladorPID pidMotor4_pitch(kp,ki,kd,false);

    volatile float ctrlM1_roll, ctrlM2_roll, ctrlM3_roll, ctrlM4_roll;
    volatile float ctrlM1_pitch, ctrlM2_pitch, ctrlM3_pitch, ctrlM4_pitch;
    volatile float ctrlM1, ctrlM2, ctrlM3, ctrlM4; //aqui sumaremos el roll y pitch
    
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

    int16_t acceleration[3], gyro[3], temp;
   
    uint32_t tiempo_previo, tiempo_actual; //esta en microsegundos( 10 ^-6)
    float dt = 0.0;
    volatile float ang_x , ang_y = 0.0;              //--INICIALIZO TODAS LAS VARIABLES A 0
    float ang_x_prev, ang_y_prev = 0.0;
    int16_t ax, ay, az = 0;
    int16_t gx, gy = 0;
    
    
    volatile float accel_ang_x = 0.0; //inicializamos a 0
    volatile float accel_ang_y = 0.0;
    
    //Añadimos el manejador para el tiempo
    sleep_ms(1000);
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
        if(sqrt((pow(ax,2) + pow(az,2))) != 0){   //compruebo division por 0
            accel_ang_x = atan(ay/sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
        }else{
            printf("Division por 0 en accel X\n");
        }
        accel_ang_y = 0.0;

        if(sqrt((pow(ay,2) + pow(az,2))) != 0){
            accel_ang_y = atan(-ax/sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
        }else{
            printf("Division por 0 en accel Y\n");
        }
        
        
        //Calculamos angulo de rotacion con giroscopio y filtro complemento
        ang_x = 0.98 * (ang_x_prev + (gx/131) * dt ) + 0.02 * accel_ang_x;
        ang_y = 0.98 * (ang_y_prev + (gy/131) * dt ) + 0.02 * accel_ang_y;
        
        ang_x_prev=ang_x;
        ang_y_prev=ang_y;

        
       //printf("%f,%f\n",ang_x,ang_y);
       
       //Calculamos las señales de control para cada motor diferenciando entre roll y pitch
       
        if(!isinf(ang_x) && !isinf(ang_y)){
            
            ctrlM1_roll = pidMotor1_roll.computar(0,ang_x);
            ctrlM1_pitch = pidMotor1_pitch.computar(0, ang_y);
            ctrlM1 = ctrlM1_roll + ctrlM1_pitch;

            ctrlM2_roll = pidMotor2_roll.computar(0,ang_x);
            ctrlM2_pitch = pidMotor2_pitch.computar(0, ang_y);
            ctrlM2 = ctrlM2_roll + ctrlM2_pitch;
            
            ctrlM3_roll = pidMotor3_roll.computar(0,ang_x);
            ctrlM3_pitch = pidMotor3_pitch.computar(0, ang_y);
            ctrlM3 = ctrlM3_roll + ctrlM3_pitch;

            ctrlM4_roll = pidMotor4_roll.computar(0,ang_x);
            ctrlM4_pitch = pidMotor4_pitch.computar(0, ang_y);
            ctrlM4 = ctrlM4_roll + ctrlM4_pitch;
           
            printf("%6.4f %6.4f \n %6.4f %6.4f \n", ctrlM1,ctrlM2,ctrlM3,ctrlM4);
        }else{
            printf("INFINITO: pitch(X): %f, roll(y): %f\n",ang_x,ang_y);
        }
        
        
        //sleep_ms(1); //Lo ideal seria esperar a la interrupcion de la MPU
    }
}