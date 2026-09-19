/* init UART: patrón literal de embeb-uart-usb/echo/main/main.c, el repo del auxiliar.
 * Armado de línea: patrón cursor (memcpy + len) de
 * embeb-uart-usb/multiplex/main/main.c
 */
#include "protocol.h"

#include <string.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/uart.h>

#define UART_PORT_NUM 0

#define RX_BUF_SIZE 2048

/* Línea completa = cuerpo + *CK + \n */
#define LINEA_MAX 96

static SemaphoreHandle_t tx_mutex;

/* Dígitos hexa mayúsculas para armar el checksum */
static const char HEXA[] = "0123456789ABCDEF";

/* Escritura cruda bajo mutex; única vía de salida por UART0 */
static void enviar_raw(const char *datos, size_t len) {
    if (tx_mutex == NULL) { /* defensa: init no llamado aún */
        uart_write_bytes(UART_PORT_NUM, datos, len);
        return;
    }
    if (xSemaphoreTake(tx_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return; /* canal ocupado: descarte silencioso */
    }
    uart_write_bytes(UART_PORT_NUM, datos, len);
    xSemaphoreGive(tx_mutex);
}

uint8_t protocolo_checksum(const char *cuerpo) {
    uint8_t ck = 0;
    while (*cuerpo) {
        ck ^= (uint8_t)*cuerpo++;
    }
    return ck;
}

void protocolo_enviar_frame(const char *datos, size_t len) {
    if (datos == NULL || len == 0 || len > LINEA_MAX - 4) {
        return; /* cuerpo ausente o demasiado largo */
    }

    /* XOR de los bytes del cuerpo */
    uint8_t ck = 0;
    for (size_t i = 0; i < len; i++) {
        ck ^= (uint8_t)datos[i];
    }

    /* Cursor: cuerpo, '*', 2 dígitos hexa, '\n' */
    char linea[LINEA_MAX];
    size_t n = 0;
    memcpy(linea + n, datos, len);
    n += len;
    linea[n++] = '*';
    linea[n++] = HEXA[(ck >> 4) & 0xF];
    linea[n++] = HEXA[ck & 0xF];
    linea[n++] = '\n';

    enviar_raw(linea, n);
}

void protocolo_enviar_linea(const char *cuerpo) {
    if (cuerpo == NULL) {
        return;
    }
    protocolo_enviar_frame(cuerpo, strlen(cuerpo));
}

/* Esta función es para probar que todo funcione. */
static void banner(void) {
    enviar_raw("tarea-1 listo (version 1.0)\n",
               sizeof("tarea-1 listo (version 1.0)\n") - 1);
}

void protocolo_init(void) {
    static const uart_config_t uart_config = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        uart_driver_install(UART_PORT_NUM, RX_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        uart_set_pin(UART_PORT_NUM, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    tx_mutex = xSemaphoreCreateMutex();

    banner();
}
