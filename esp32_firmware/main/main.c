/* Arranque: protocolo UART0 + síntesis de señales por eje + ambiente */
#include "protocol.h"
#include "sensors.h"
#include "ambiente.h"

void app_main(void) {
    protocolo_init();
    sensors_init();
    ambiente_init();
}
