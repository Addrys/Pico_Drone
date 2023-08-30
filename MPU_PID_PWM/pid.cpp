#include "pid.h"

controladorPID::controladorPID(float kp, float ki, float kd, bool polaridad)
    : kp(kp), ki(ki), kd(kd), polaridad(polaridad),
      prevError(0.0), integral(0.0) {}

float controladorPID::computar(float setpoint, float current) {
    if (!polaridad){current = -current;}
    float error = setpoint - (current);
    integral += error;
    float derivative = error - prevError;
    prevError = error;

    float output = kp * error + ki * integral + kd * derivative;
    return output;
}