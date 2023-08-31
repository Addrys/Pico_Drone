#include "pid.h"

controladorPID::controladorPID(float kp, float ki, float kd, bool polaridad)
    : kp(kp), ki(ki), kd(kd), polaridad(polaridad),
      prevError(0.0), integral(0.0) {}

float controladorPID::computar(float setpoint, float current){
    if (!polaridad){current = -current;}
    float error = setpoint - current;
    integral = integral + error * 0.003; // Ts = 3ms (tiempo de muestreo)
    integral = 0.0;
    //Reparar controlador PID
    float output = kp*(error + ((1/ki)*integral) + (kd*(error-prevError)/0.003));
    prevError = error;
    if(output < 0.0){output = 0;}
    return output;
}
/*
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
    //vamos a quitar las señales negativas a ver que tal afecta, sino disminuirlas
    if(output < 0.0){output = 0.0;}
    
    return output;
    
}*/