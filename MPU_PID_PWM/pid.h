#ifndef PID_H
#define PID_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include <queue>

class controladorPID {
public:
    controladorPID(float kp, float ki, float kd, bool polaridad);
    float computar(float setpoint, float current);
    void reiniciar_integrador();

private:
    float kp, ki, kd;
    bool polaridad;
    float prevError;
    float integral;
    std::queue<float> buffer_integrador;
    uint8_t integrador_size;
    
};

#endif // PID_H