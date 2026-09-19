/* Contrato: líneas ASCII terminadas en '\n', argumentos separados por comas,
 * checksum XOR de 2 dígitos hexa mayúsculas tras '*'.
 * Cuerpos inválidos o demasiado largos se descartan en silencio.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

/* Configura UART0 (TX GPIO1 / RX GPIO3, 115200 8N1), instala el driver,
 * crea el mutex TX y emite texto en el arranque con comandos checksumados. */
void protocolo_init(void);

/* Transmite <datos>*CK\n: armado usando un cursor y varios memcpy+len.
 * Evita dataraces entre tasks usando un mutex; len no incluye '\0' final. */
void protocolo_enviar_frame(const char *datos, size_t len);

/* Conveniencia para cuerpos terminados en '\0': delega en
 * protocolo_enviar_frame() midiendo con strlen. */
void protocolo_enviar_linea(const char *cuerpo);

/* Checksum con XOR de todos los bytes del cuerpo (sin '*' ni checksum). */
uint8_t protocolo_checksum(const char *cuerpo);

#endif /* PROTOCOL_H */
