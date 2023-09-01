#include "mpu.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>
#include <time.h>
#include "pid.h"
#include "pwm.h"
#include "hardware/timer.h"

#include "hardware/adc.h"

static int addr = 0x68;
int write_block,read_block = 0;

#define ACCEL_SENSITIVITY 16384.0  // Sensibilidad del acelerómetro en LSB/g
#define GYRO_SENSITIVITY 131.0   


//para medir tiempos 
volatile uint32_t actual = 0, pasado = 0, tiempo_pasado;

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
    adc_init();
    adc_gpio_init(26);

    //Ajustamos los coeficientes del controlador PID
    float kp = 0.1;
    float ki = 0.003; //0.1
    float kd = 0.002; //1.0
    //creamos los controladores
    controladorPID pidMotor1_roll(kp,ki,kd,true);
    controladorPID pidMotor2_roll(kp,ki,kd,true);//Roll + para M2
    controladorPID pidMotor3_roll(kp,ki,kd,false);
    controladorPID pidMotor4_roll(kp,ki,kd,false);
    controladorPID pidMotor1_pitch(kp,ki,kd,true);
    controladorPID pidMotor2_pitch(kp,ki,kd,false);//Pitch - para M2
    controladorPID pidMotor3_pitch(kp,ki,kd,true);
    controladorPID pidMotor4_pitch(kp,ki,kd,false);

    //PWM 0,7pi,9,13
    controladorPWM pwm_motor1(19);
    controladorPWM pwm_motor2(7);
    controladorPWM pwm_motor3(20);
    controladorPWM pwm_motor4(0);

    pwm_motor1.inicializar();
    pwm_motor2.inicializar();
    pwm_motor3.inicializar();
    pwm_motor4.inicializar();
    

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

    uint16_t valor = 0;
    float us_deseados;

    uint16_t level = 480, us_m1, us_m2, us_m3, us_m4;
    while (1) {
        
        mpu6050_read_raw(acceleration, gyro, &temp);
        //mostrar_offsets();
        
        ax = acceleration[0]; ay = acceleration[1]; az = acceleration[2];
        gx = gyro[0]; gy = gyro[1]; 
        
         clock_t tiempo_actual = clock();
        dt = (float)(tiempo_actual - tiempo_previo) / CLOCKS_PER_SEC;
        tiempo_previo = tiempo_actual;

        // Calcula los ángulos con el acelerómetro
        float accel_magnitude = sqrt(ax * ax + ay * ay + az * az);
        if (accel_magnitude != 0) {
            accel_ang_x = atan2(ay, az) * (180.0 / M_PI);
            accel_ang_y = atan2(-ax, sqrt(ay * ay + az * az)) * (180.0 / M_PI);
        } else {
            printf("División por cero en el cálculo del ángulo con el acelerómetro\n");
        }

        // Calcula el ángulo de rotación con el giroscopio y filtro complementario
        ang_x = 0.98 * (ang_x + (gx / GYRO_SENSITIVITY) * dt) + 0.02 * accel_ang_x;
        ang_y = 0.98 * (ang_y + (gy / GYRO_SENSITIVITY) * dt) + 0.02 * accel_ang_y;

        //printf("%f, %f\n", ang_x, ang_y);
       
       //Calculamos las señales de control para cada motor diferenciando entre roll y pitch
       
        if(!isinf(ang_x) && !isinf(ang_y)){
            
            ctrlM1_roll = pidMotor1_roll.computar(0,ang_x);
            ctrlM1_pitch = pidMotor1_pitch.computar(0, ang_y);
            ctrlM1 = (ctrlM1_roll + ctrlM1_pitch)/2;

            ctrlM2_roll = pidMotor2_roll.computar(0,ang_x);
            ctrlM2_pitch = pidMotor2_pitch.computar(0, ang_y);
            ctrlM2 = (ctrlM2_roll + ctrlM2_pitch)/2;
            
            ctrlM3_roll = pidMotor3_roll.computar(0,ang_x);
            ctrlM3_pitch = pidMotor3_pitch.computar(0, ang_y);
            ctrlM3 = (ctrlM3_roll + ctrlM3_pitch)/2;

            ctrlM4_roll = pidMotor4_roll.computar(0,ang_x);
            ctrlM4_pitch = pidMotor4_pitch.computar(0, ang_y);
            ctrlM4 = (ctrlM4_roll + ctrlM4_pitch)/2;
           
           printf("%6.4f %6.4f \n %6.4f %6.4f \n", ctrlM1,ctrlM2,ctrlM3,ctrlM4);
            

            uint16_t raw_value = adc_read();

            if(raw_value <= 500){
                raw_value = 500;
            }else if(raw_value >= 4000){
                raw_value = 4000;
                }
        

            valor =((raw_value-500)/35); //Valor = [0,100] en funcion del potenciometro
           // printf("valor: %d\n",valor);
            us_deseados = 800 + 10*valor;

            //us_deseados = 100 + (valor);
            
            us_m1 = us_deseados;// + (ctrlM1);
            us_m2 = us_deseados;// + (ctrlM2);
            us_m3 = us_deseados*0.98;// + (ctrlM3);
            us_m4 = us_deseados;// + (ctrlM4);

            /*
            level_m1 = (us_deseados);
            pwm_motor1.controlar(level_m1);

            level_m2 = (1000*ms_m2);
            level_m3 = (1000*ms_m3);
            level_m4 = (1000*ms_m4);*/
            if(us_m1 < 0 || us_m1 > 2000){
                us_m1 = 0;
            }
            if(us_m2 < 0 || us_m2 > 2000){
                us_m2 = 0;
            }
            if(us_m3 < 0 || us_m3 > 2000){
                us_m3 = 0;
            }
            if(us_m4 < 0 || us_m4 > 2000){
                us_m4 = 0;
            }
            
            //printf("\n%f\n",us_deseados);
            if(us_m1 < 2000 && us_m1 > -2000){
              //  printf("\n%d %d \n %d %d \n", us_m1,us_m2,us_m3,us_m4);
                //printf("\n valor: %d", us_m1);
                /*
                if(us_m1 < 1000){us_m1 = 1000;}
                if(us_m2 < 1000){us_m2 = 1000;}
                if(us_m3 < 1000){us_m3 = 1000;}
                if(us_m4 < 1000){us_m4 = 1000;}
                */
               //printf("\n\t\tAcelerador: %d", us_m1);
                pwm_motor1.controlar(us_m1);
                pwm_motor2.controlar(us_m2);
                pwm_motor3.controlar(us_m3);
                pwm_motor4.controlar(us_m4);
                //actual = time_us_32();
               // tiempo_pasado = actual - pasado;
              //  printf("Tiempo desde el último pulso: %u microsegundos\n", tiempo_pasado);
                //pasado = actual;
            }else{
                printf("%f Calculando...\n", us_m1);
            }
        
        }else{
            printf("INFINITO: pitch(X): %f, roll(y): %f\n",ang_x,ang_y);
        }
        
        
        //sleep_ms(1); //Lo ideal seria esperar a la interrupcion de la MPU
    }
}