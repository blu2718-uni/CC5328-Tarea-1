 /* temperatura 15.0–30.0 °C y humedad relativa 20–40 %. */
#ifndef AMBIENTE_H
#define AMBIENTE_H

/* Crea la task que emite AMBIENTE,<°C>,<%HR>*CK cada 30 s
 * vía protocolo (protocol.h). */
void ambiente_init(void);

#endif /* AMBIENTE_H */
