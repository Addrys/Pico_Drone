#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

#include <stdio.h>

constexpr uint PIN_OUT = 0;

int main()
{
    //preparacion para el potenciometro
    stdio_init_all();
    sleep_ms(2000);   
    adc_init();
    adc_gpio_init(26);

    gpio_set_function(PIN_OUT, GPIO_FUNC_PWM);  /// Set the pin 0 to be PWM
    auto slice   = pwm_gpio_to_slice_num(PIN_OUT);
    auto channel = pwm_gpio_to_channel(PIN_OUT);

    #define LED_PIN 16
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    pwm_set_clkdiv(slice, 256.0f);  /// Setting the divider to slow down the clock
    pwm_set_wrap(slice, 32550);      ///wrap para los 67ms 
    pwm_set_enabled(slice, true);

    //vamos a salir de los limities un poco llendo desde menos de 1000us (nivel480) a algo más de 2000us (nivel 1000)
    uint16_t level = 480;

    bool ascenso = true;
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

        if(us_deseados > 1.8){ us_deseados = us_deseados * 1.1;}
        level = (489*us_deseados);
        pwm_set_chan_level(slice, channel, level); 
       // printf("RAW: %d\t Valor: %d\t level: %d\t, ms:%f\n",raw_value,valor,level,us_deseados);


       // sleep_ms(100);
        

    }
    return 0;
}
//https://cocode.se/linux/raspberry/pwm.html#orgf2a00a2