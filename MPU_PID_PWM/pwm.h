#ifndef PWM_H
#define PWM_H

#include "hardware/pwm.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

class controladorPWM {
public:
    controladorPWM(int gpio_pwm);
    void inicializar();
    void controlar(uint16_t signal); //La señal a enviar (1000-2000us)

private:
    int gpio_pwm;
    uint slice, channel;
};

#endif // PWM_H