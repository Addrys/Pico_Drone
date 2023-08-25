#include "mpu.h"
#include "pid.h"


int main() {
    //Inicializamos la MPU
    stdio_init_all();
    sleep_ms(5000);
    mpu6050_run();

    return 0;
}