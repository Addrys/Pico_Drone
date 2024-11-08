#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"

#include "cgi.h"

#include "ssi.h"
#include "mpu.h"

// WIFI Credentials - take care if pushing to github!
const char WIFI_SSID[] = "MOVISTAR_5433";
const char WIFI_PASSWORD[] = "SONIAYDANY05";

#define PIN_INTERRUPTION 0
int main() {
    stdio_init_all();

    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    sleep_ms(3000);
    // Connect to the WiFI network - loop until connected
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0){
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected! \n");
    
    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");

    // Configure SSI and CGI handler
 

    float ang_x, ang_y = 0.0;
    float* ang_x_ref = &ang_x;
    float* ang_y_ref = &ang_y;

    printf("Direccion ANG_X en MAIN1 : %p \n",(void*)ang_x_ref);

    ssi_init(ang_x_ref,ang_y_ref); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    printf("Direccion ANG_X en MAIN2 : %p \n",(void*)ang_x_ref);
    
    mpu_run();

    // Configura el pin de interrupción como entrada
/*
    gpio_init(PIN_INTERRUPTION);
    gpio_set_dir(PIN_INTERRUPTION, GPIO_IN);

    // Configura la interrupción de cambio de nivel en el pin
    gpio_set_irq_enabled_with_callback(PIN_INTERRUPTION, GPIO_IRQ_EDGE_FALL, true, ssi_handler);
*/

    // Infinite loop
    printf("ERROR FINALIZADO######################");
    while(1);
}