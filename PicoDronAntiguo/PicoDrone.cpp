#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

constexpr uint PIN_OUT = 0;

int main()
{
    stdio_init_all();
    sleep_ms(4000);                             /// Just to wait some..
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
    int level = 480;
    bool ascenso = true;
    while(true){
        
        if(ascenso == false){
            gpio_put(LED_PIN, 0);
            level -=1;
            if(level <= 480){
                level = 480;
                ascenso = true;
                sleep_ms(3000);
            }
        }else{
            gpio_put(LED_PIN, 1);
            level += 1;
            if(level >= 980){
                level = 980; 
                ascenso = false;
            }
        }
        pwm_set_chan_level(slice, channel, level); 
        sleep_ms(2);
    }
    return 0;
}
//https://cocode.se/linux/raspberry/pwm.html#orgf2a00a2