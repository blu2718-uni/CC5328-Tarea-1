#include <stdio.h>
#include <stdint.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define AMPLITUD_G 4.0f

#define PERIODO_MS 100

static void tarea_datos(void *arg) {
    (void)arg;
    TickType_t ultimo = xTaskGetTickCount();
    while (1) {
        /* Un valor vivo por eje */
        printf("ACELEROMETRO,X,%.3f\n",
               (esp_random() / (float)UINT32_MAX) * 2.0f * AMPLITUD_G - AMPLITUD_G);
        printf("ACELEROMETRO,Y,%.3f\n",
               (esp_random() / (float)UINT32_MAX) * 2.0f * AMPLITUD_G - AMPLITUD_G);
        printf("ACELEROMETRO,Z,%.3f\n",
               (esp_random() / (float)UINT32_MAX) * 2.0f * AMPLITUD_G - AMPLITUD_G);
        vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(PERIODO_MS));
    }
}

void app_main(void) {
    xTaskCreate(tarea_datos, "datos", 4096, NULL, 10, NULL);
}
