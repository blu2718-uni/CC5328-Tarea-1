/* Emite ACELEROMETRO,<eje>,<valor>*CK cada 100 ms */
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "protocol.h"

#define AMPLITUD_G 4.0f

/* Periodo de emisión */
#define PERIODO_MS 100

#define ETIQUETA "ACELEROMETRO,"
#define ETIQUETA_LEN (sizeof(ETIQUETA) - 1)

static void agregar_milig(char *buf, size_t *len, int32_t milig) {
    if (milig < 0) {
        buf[(*len)++] = '-';
        milig = -milig;
    }
    uint32_t entero = (uint32_t)milig / 1000;
    uint32_t frac = (uint32_t)milig % 1000;
    char digitos[10];
    size_t nd = 0;
    do {
        digitos[nd++] = (char)('0' + entero % 10);
        entero /= 10;
    } while (entero != 0);
    while (nd != 0) {
        buf[(*len)++] = digitos[--nd];
    }
    buf[(*len)++] = '.';
    buf[(*len)++] = (char)('0' + frac / 100);
    buf[(*len)++] = (char)('0' + (frac / 10) % 10);
    buf[(*len)++] = (char)('0' + frac % 10);
}

static void tarea_datos(void *arg) {
    (void)arg;
    TickType_t ultimo = xTaskGetTickCount();
    static const char EJES[] = {'X', 'Y', 'Z'};
    char buf[24];
    while (1) {
        /* Un valor vivo por eje */
        for (size_t i = 0; i < 3; i++) {
            float valor = (esp_random() / (float)UINT32_MAX) * 2.0f * AMPLITUD_G
                          - AMPLITUD_G;
            size_t len = 0;
            memcpy(buf + len, ETIQUETA, ETIQUETA_LEN);
            len += ETIQUETA_LEN;
            buf[len++] = EJES[i];
            buf[len++] = ',';
            agregar_milig(buf, &len, lroundf(valor * 1000.0f));
            protocolo_enviar_frame(buf, len);
        }
        vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(PERIODO_MS));
    }
}

void app_main(void) {
    protocolo_init();
    xTaskCreate(tarea_datos, "datos", 4096, NULL, 10, NULL);
}
