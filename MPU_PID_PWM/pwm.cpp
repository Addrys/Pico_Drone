#include "pwm.h"

controladorPWM::controladorPWM(int gpio_pwm)
    : gpio_pwm(gpio_pwm), slice(0), channel(0) {}

void controladorPWM::inicializar(){
    //Inicialización del canal PWM
    gpio_set_function(gpio_pwm, GPIO_FUNC_PWM); // Seleccionamos el pin gpio_pwm para ser un canal PWM
    slice = pwm_gpio_to_slice_num(gpio_pwm);
    channel = pwm_gpio_to_channel(gpio_pwm);

    pwm_set_clkdiv(slice, 125.0f); //Divisor para frenar el clk del RP2040 (máx valor 8 bits = 256)
    pwm_set_wrap(slice, 3000);
    pwm_set_enabled(slice, true);

    pwm_set_chan_level(slice, channel, 1000); // mandamos 1000us para activar el motor
}

void controladorPWM::controlar(uint16_t signal){
    //mandar la señal deseada por el canal PWM

    pwm_set_chan_level(slice, channel, signal);
}
