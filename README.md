# CC5328-Tarea-1
Demo: https://youtu.be/hoQxYrGXY4g

## Firmware: build y flash

En Linux:

```bash
cd esp32_firmware
source ~/esp/esp-idf/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

- Al arrancar emite una línea de salud: `tarea-1 listo (version 1.0)`.
- Con el firmware sin GUI se puede probar a mano: el monitor acepta pegar comandos (los checksums de abajo van listos).

```
INICIAR*57
AMPLITUD,X,4*70
FRECUENCIA,X,50*5A
PERIODO,30*65
BAUDIOS,230400*6E
FUNCION,X,1*3F
```

## Protocolo serial

Líneas ASCII terminadas en `\n`, campos separados por **comas**, checksum **XOR-8** de todos los bytes del cuerpo (lo que va antes del `*`), expresado en 2 dígitos hexa mayúsculas después de `*`. Una entrada sin ese checksum (o una que no esté bien formada) se **descarta en silencio**.

### Datos (ESP → PC)

```
ACELEROMETRO,<eje>,<valor>*CK    eje: X|Y|Z; valor en g, formato d.ddd
AMBIENTE,<°C>,<%HR>*CK           °C con 1 decimal (15.0–30.0); HR entero (20–40)
```

### Comandos (PC → ESP)

Todo comando recibe `OK,<cmd>,<detalle>*CK` o `ERROR,<cmd>,<motivo>*CK` por parte del ESP. Un comando con checksum válido pero desconocido responde `ERROR,desconocido,sin soporte`.

| Comando | Válidos | Efecto |
|---|---|---|
| `INICIAR` | sin argumentos | defaults (función 1, 4 g, 100 Hz), fase 0, ambiente 30 s y baudio 115200 |
| `FUNCION,<eje>,<n>` | 1 armónica · 2 modulada en amplitud · 3 multicomponente | cambia la señal del eje |
| `AMPLITUD,<eje>,<g>` | 4 · 8 · 16 | amplitud del eje en g |
| `FRECUENCIA,<eje>,<fs>` | 50 · 100 · 200 · 500 · 1000 | fs de **muestreo** del eje, no la frecuencia de la señal |
| `PERIODO,<s>` | 30 · 60 | periodo de emisión de AMBIENTE |
| `BAUDIOS,<b>` | 115200 · 230400 · 460800 · 921600 | hot-swap del baudio en caliente |

## GUI

El repo contempla `pip install -r requirements.txt` y `python gui_python/main.py` (desde la raíz, estando en el entorno virtual); este repo además usa [uv](https://docs.astral.sh/uv/):

```bash
# vía uv
cd gui_python && uv sync && uv run python main.py

# vía pip
source gui_python/.venv/bin/activate
pip install -r requirements.txt
python gui_python/main.py
```

- **Menú Conexión**: Conectar , Desconectar e Inicializar (setea los valores por defecto de la conexion y los de cada eje).
- **Gráficos**: ventana deslizante de 5 s por eje (redibujo a 30 fps). Botón *Pausar lecturas* congela los gráficos sin dejar de drenar el puerto.
- **Estadísticas de ambiente**: indicadores numéricos de T y HR con lo que llega del ESP.
- **Configuración**: puerto (detectado automáticamente, testeado en linux, solo USB/ACM), baudio y periodo de ambiente.
