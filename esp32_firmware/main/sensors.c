/* Sintetiza las señales de vibración por eje y las emite como
 * ACELEROMETRO,<eje>,<valor>*CK.
 * Fórmulas (Tarea1_CC5328_v2.md 2.1.1):
 *   1) A·sin(2πft)
 *   2) A·cos(2πf1t)·sin(2πf2t)
 *   3) (2A/2)·[sin(2πft) + cos(4πft)] = A·[sin(2πft) + cos(4πft)]
 */
#include "sensors.h"
#include "protocol.h"

#include <math.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define PI 3.14159265358979f

/* Etiqueta de dato en el contrato */
#define ETIQUETA "ACELEROMETRO,"
#define ETIQUETA_LEN (sizeof(ETIQUETA) - 1)

/* Configuración por eje; la escriben los setters de P5 */
static sensor_config_t config[SENSOR_EJES];

/* Default: función 1, 4 g, 100 Hz */
sensor_config_t sensor_config_default(void) {
    return (sensor_config_t){
        .funcion = SENSOR_FUNCION_ARMONICA,
        .amplitud_g = 4,
        .fs_hz = 100,
    };
}

/* Evalúa la función del eje en la muestra n; t = n/fs */
static float valor_funcion(const sensor_config_t *c, uint32_t n) {
    float t = (float)n / (float)c->fs_hz;
    if (c->funcion == SENSOR_FUNCION_MODULADA) {
        return (float)c->amplitud_g
               * cosf(2.0f * PI * SENSOR_F1_HZ * t)
               * sinf(2.0f * PI * SENSOR_F2_HZ * t);
    }
    if (c->funcion == SENSOR_FUNCION_MULTICOMPONENTE) {
        /* cos(4πft) = cos(2π·2f·t) */
        return (float)c->amplitud_g
               * (sinf(2.0f * PI * SENSOR_F_HZ * t)
                  + cosf(2.0f * PI * 2.0f * SENSOR_F_HZ * t));
    }
    /* función 1 (armónica simple) es el default */
    return (float)c->amplitud_g * sinf(2.0f * PI * SENSOR_F_HZ * t);
}

/* arg = eje; task creation y stack de embeb-uart-usb/echo/main/main.c */
static void tarea_sensor(void *arg) {
    sensor_eje_t eje = (sensor_eje_t)(size_t)arg;
    static const char EJES[] = {'X', 'Y', 'Z'};
    TickType_t ultimo = xTaskGetTickCount();
    char buf[24];
    uint32_t muestra = 0;
    while (1) {
        TickType_t periodo = pdMS_TO_TICKS(1000 / config[eje].fs_hz);
        float valor = valor_funcion(&config[eje], muestra++);
        size_t len = 0;
        memcpy(buf + len, ETIQUETA, ETIQUETA_LEN);
        len += ETIQUETA_LEN;
        buf[len++] = EJES[eje];
        buf[len++] = ',';
        protocolo_agregar_milig(buf, &len, lroundf(valor * 1000.0f));
        protocolo_enviar_frame(buf, len);
        vTaskDelayUntil(&ultimo, periodo);
    }
}

/* Default de configuración y 3 tasks (X, Y y Z) */
void sensors_init(void) {
    for (size_t i = 0; i < SENSOR_EJES; i++) {
        config[i] = sensor_config_default();
    }
    for (size_t i = 0; i < SENSOR_EJES; i++) {
        xTaskCreate(tarea_sensor, "sensor", 4096, (void *)(size_t)i, 10, NULL);
    }
}
