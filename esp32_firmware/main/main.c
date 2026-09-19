/* Arranque: protocolo UART0 + síntesis de señales por eje */
#include "protocol.h"
#include "sensors.h"

void app_main(void) {
    protocolo_init();
    sensors_init();
}
