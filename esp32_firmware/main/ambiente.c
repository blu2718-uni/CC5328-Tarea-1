/* Emite AMBIENTE,<°C>,<%HR>*CK.
 * Valores sintéticos con esp_random
 */
#include "ambiente.h"
#include "protocol.h"

#include <math.h>
#include <string.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* Rangos del enunciado */
#define TEMP_MIN 15.0f
#define TEMP_MAX 30.0f
#define HR_MIN 20
#define HR_MAX 40

#define ETIQUETA "AMBIENTE,"
#define ETIQUETA_LEN (sizeof(ETIQUETA) - 1)

/* Periodo de emisión en segundos */
static uint32_t periodo_s = 30;

/* Agrega un entero (con signo) al cursor; misma técnica de dígitos
 * en reversa que protocolo_agregar_milig */
static void agregar_entero(char *buf, size_t *len, int32_t valor) {
    uint32_t sin_signo = (valor < 0) ? (uint32_t)(-valor) : (uint32_t)valor;
    if (valor < 0) {
        buf[(*len)++] = '-';
    }
    char digitos[10];
    size_t nd = 0;
    do {
        digitos[nd++] = (char)('0' + sin_signo % 10);
        sin_signo /= 10;
    } while (sin_signo != 0);
    while (nd != 0) {
        buf[(*len)++] = digitos[--nd];
    }
}

/* task creation. */
static void tarea_ambiente(void *arg) {
    (void)arg;
    TickType_t ultimo = xTaskGetTickCount();
    char buf[24];
    while (1) {
        /* Temperatura en décimas uniformes (150..300) */
        int32_t temp_dec = lroundf((TEMP_MIN
                                    + (esp_random() / (float)UINT32_MAX)
                                          * (TEMP_MAX - TEMP_MIN))
                                   * 10.0f);
        /* Humedad entera uniforme (20..40) */
        int32_t hr = HR_MIN + (esp_random() % (HR_MAX - HR_MIN + 1));

        size_t len = 0;
        memcpy(buf + len, ETIQUETA, ETIQUETA_LEN);
        len += ETIQUETA_LEN;
        agregar_entero(buf, &len, temp_dec / 10);
        buf[len++] = '.';
        agregar_entero(buf, &len, temp_dec % 10);
        buf[len++] = ',';
        agregar_entero(buf, &len, hr);
        protocolo_enviar_frame(buf, len);
        vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(periodo_s * 1000));
    }
}

void ambiente_init(void) {
    xTaskCreate(tarea_ambiente, "ambiente", 4096, NULL, 10, NULL);
}
