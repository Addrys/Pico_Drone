#include "pid.h"

controladorPID::controladorPID(float kp, float ki, float kd, bool polaridad)
    : kp(kp), ki(ki), kd(kd), polaridad(polaridad),
      prevError(0.0), integral(0.0), integrador_size(20) {}

float controladorPID::computar(float setpoint, float current){
    if (!polaridad){current = -current;}
    float error = setpoint - current;
   // integral = integral + error * 0.003; // Ts = 3ms (tiempo de muestreo)
   // integral = 0.0;

//rellenamos la cola
    if(buffer_integrador.size() >= integrador_size){
        //Si ya esta llena la coola
        buffer_integrador.pop(); //sacamos el elemento más viejo
        buffer_integrador.push(error); //metemos nuestro nuevo error
    }else{
        //Si estamos llenando la cola
        buffer_integrador.push(error);
    }

//hacemos el sumatorio de los elementos en el buffer integrador
    //cola copia
    std::queue<float> copia_buffer = buffer_integrador;
    float sumatorio = 0.0;
    while(!copia_buffer.empty()){
        sumatorio += copia_buffer.front();
        copia_buffer.pop();
    }
//calculamos el valor del integrador
    integral = sumatorio * 0.003;
   // printf("\n Integrador: %f\n",((1/ki)*integral));
    //Reparar controlador PID
    float output = kp*(error + ((1/ki)*integral) + (kd*(error-prevError)/0.003));
    prevError = error;
     //if(output < 0.0){output = 0;}
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