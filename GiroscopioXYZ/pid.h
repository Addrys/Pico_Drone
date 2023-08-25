#ifndef PID_H
#define PID_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

class controladorPID {
public:
    controladorPID(float kp, float ki, float kd, bool polaridad);
    float computar(float setpoint, float current);

private:
    float kp, ki, kd, polaridad;
    float prevError;
    float integral;
};

#endif // PID_H