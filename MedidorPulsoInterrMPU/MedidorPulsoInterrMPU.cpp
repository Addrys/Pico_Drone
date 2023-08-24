#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <cstdio>

#define PULSE_PIN 1

volatile uint32_t last_pulse_time = 0;

void pulse_handler(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    uint32_t time_since_last_pulse = current_time - last_pulse_time;
    last_pulse_time = current_time;
    printf("Tiempo desde el último pulso: %u microsegundos\n", time_since_last_pulse);
}

int main() {
    stdio_init_all();

    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_IN);
    
    // Configuración del timer para generar interrup

    // Configuración de la interrupción para el pin de pulso
    gpio_set_irq_enabled_with_callback(PULSE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &pulse_handler);

    while (1) {
        tight_loop_contents();
    }

    return 0;
}
