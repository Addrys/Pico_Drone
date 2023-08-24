#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>
#include <time.h>
#include "mpu.h"

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"volt","temp","led"};
float *referencia_ang_x, *referencia_ang_y;
u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;
  switch (iIndex) {
    case 0: // X
      { 
        //printf("RUNNING MPU");
        //float ang_x, ang_y;
        //mpu_run(&ang_x,&ang_y);
        //printf("%f,%f\n",ang_x,ang_y);
        const float ang_x_static = *referencia_ang_x;
        printf("%f\t",ang_x_static);
        printed = snprintf(pcInsert, iInsertLen, "%f", ang_x_static);
      }
      break;
    case 1: // Y
      {
        
        const float ang_y_static = *referencia_ang_y;
        printf("%f\n",ang_y_static);
        printed = snprintf(pcInsert, iInsertLen, "%f", ang_y_static);
      }
      break;
    case 2: // led
      {
        bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
        if(led_status == true){
          printed = snprintf(pcInsert, iInsertLen, "ON");
        }
        else{
          printed = snprintf(pcInsert, iInsertLen, "OFF");
        }
      }
      break;
    default:
      printed = 0;
      break;
  }

  return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init(float* ang_x_ref, float* ang_y_ref) {
  // Initialise ADC (internal pin)
  adc_init();
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);
  //iniciamos las configutacion del MPU
  mpu_init(ang_x_ref,ang_y_ref);
  referencia_ang_x = ang_x_ref;
  referencia_ang_y = ang_y_ref;
  printf("MPU iniciada con exito:\n");
  printf("Direccion ANG_X en SSI : %p \n",(void*)ang_x_ref);


  
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}