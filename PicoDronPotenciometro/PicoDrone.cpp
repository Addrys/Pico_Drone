#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

#include <stdio.h>

#define PWM1 0
#define PWM2 5
#define PWM3 9
#define PWM4 13

int main()
{
    //preparacion para el potenciometro
    stdio_init_all();
    sleep_ms(2000);   
    adc_init();
    adc_gpio_init(26);

    #define LED_PIN 16
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

//--------------DEFINIMOS LOS CANELES PWM -----------
    gpio_set_function(PWM1, GPIO_FUNC_PWM);  /// Set the pin 0 to be PWM
    auto slice_1   = pwm_gpio_to_slice_num(PWM1);
    auto channel_1 = pwm_gpio_to_channel(PWM1);

    pwm_set_clkdiv(slice_1, 256.0f);  /// Setting the divider to slow down the clock
    pwm_set_wrap(slice_1, 32550);      ///wrap para los 67ms 
    pwm_set_enabled(slice_1, true);
//-----------------------------------------------------------
    gpio_set_function(PWM2, GPIO_FUNC_PWM);  /// Set the pin 0 to be PWM
    auto slice_2   = pwm_gpio_to_slice_num(PWM2);
    auto channel_2 = pwm_gpio_to_channel(PWM2);

    pwm_set_clkdiv(slice_2, 256.0f);  /// Setting the divider to slow down the clock
    pwm_set_wrap(slice_2, 32550);      ///wrap para los 67ms 
    pwm_set_enabled(slice_2, true);
//-----------------------------------------------------------------
    gpio_set_function(PWM3, GPIO_FUNC_PWM);  /// Set the pin 0 to be PWM
    auto slice_3   = pwm_gpio_to_slice_num(PWM3);
    auto channel_3 = pwm_gpio_to_channel(PWM3);

    pwm_set_clkdiv(slice_3, 256.0f);  /// Setting the divider to slow down the clock
    pwm_set_wrap(slice_3, 32550);      ///wrap para los 67ms 
    pwm_set_enabled(slice_3, true);
//---------------------------------------------------------------
    gpio_set_function(PWM4, GPIO_FUNC_PWM);  /// Set the pin 0 to be PWM
    auto slice_4   = pwm_gpio_to_slice_num(PWM4);
    auto channel_4 = pwm_gpio_to_channel(PWM4);

    pwm_set_clkdiv(slice_4, 256.0f);  /// Setting the divider to slow down the clock
    pwm_set_wrap(slice_4, 32550);      ///wrap para los 67ms 
    pwm_set_enabled(slice_4, true);
//..------------------------------------------------------------

    //vamos a salir de los limities un poco llendo desde menos de 1000us (nivel480) a algo más de 2000us (nivel 1000)
    uint16_t level = 480;

    gpio_put(LED_PIN, 1);

    uint16_t valor = 0;
    float us_deseados;
    while(true){
        //Leemos el valor raw del potenciometro
        uint16_t raw_value = adc_read();
        
        //lo convertimos al rango 0-255
      //  uint8_t valor = static_cast<uint8_t>(raw_value >> 8);
        
        

        
        

        if(raw_value <= 500){
            raw_value = 500;
            }else if(raw_value >= 4000){
                raw_value = 4000;
                }
        

        valor =((raw_value-500)/35);
        //uint16_t us_deseados = 1000 + valor*4;
        us_deseados = 100 + (valor);
        us_deseados = us_deseados/100;

        level = (489*us_deseados);
        pwm_set_chan_level(slice_1, channel_1, level); 
        pwm_set_chan_level(slice_2, channel_2, level); 
        pwm_set_chan_level(slice_3, channel_3, level); 
        pwm_set_chan_level(slice_4, channel_4, level); 
        
       // printf("RAW: %d\t Valor: %d\t level: %d\t, ms:%f\n",raw_value,valor,level,us_deseados);


       // sleep_ms(100);
        

    }
    return 0;
}
//https://cocode.se/linux/raspberry/pwm.html#orgf2a00a2