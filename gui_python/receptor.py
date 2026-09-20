"""Receptor serial: QObject movido a un QThread que sondea el puerto,
valida las líneas con protocol.validar() y las reparte como señales Qt.
"""
import queue

import serial
from PyQt6.QtCore import QObject, QTimer, pyqtSignal, pyqtSlot

import protocol

class ReceptorSerial(QObject):
    dato_eje = pyqtSignal(str, float)  # eje X/Y/Z, valor en g
    ambiente = pyqtSignal(float, int)  # °C, %HR
    respuesta = pyqtSignal(str)  # cuerpo completo OK/ERROR
    conexion = pyqtSignal(bool, str)  # estado + motivo del fallo

    def __init__(self, puerto, baudios):
        super().__init__()
        self.serie = None
        self.puerto = puerto
        self.pausado = False
        self.cola_tx = queue.Queue()
        self.buffer = ""
        self._baudios = baudios
        self._baudios_pendientes = None
        self.timer = QTimer(self)
        self.timer.setInterval(20)
        self.timer.timeout.connect(self.leer)

    @pyqtSlot()
    def arrancar(self):
        try:
            self.serie = serial.Serial(self.puerto, self._baudios, timeout=0)
        except serial.SerialException as motivo:
            self.conexion.emit(False, str(motivo))
            return
        self.conexion.emit(True, "")
        self.serie.write(protocol.comando_iniciar().encode("ascii"))
        self.timer.start()

    @pyqtSlot()
    def leer(self):
        if self.serie is None or not self.serie.is_open:
            return
        self.chequear_baudios()
        self.escribir_pendientes()
        n = self.serie.in_waiting
        if n == 0:
            return
        self.buffer += self.serie.read(n).decode(errors="replace")
        if len(self.buffer) > 4096:
            self.buffer = ""
            return
        lineas = self.buffer.split("\n")
        self.buffer = lineas.pop()
        for linea in lineas:
            cuerpo = protocol.validar(linea)
            if cuerpo is None:
                continue
            self.repartir(cuerpo)

    def chequear_baudios(self):
        """Si la GUI pidió otro baudio (comando BAUDIOS ya enviado), cierra
        y reabre el puerto a la velocidad nueva."""
        if self._baudios_pendientes is None:
            return
        self._baudios = self._baudios_pendientes
        self._baudios_pendientes = None
        self.serie.close()
        self.serie = serial.Serial(self.puerto, self._baudios, timeout=0)

    def escribir_pendientes(self):
        """Comandos encolados desde la GUI (escritura solo en este hilo)."""
        while True:
            try:
                texto = self.cola_tx.get_nowait()
            except queue.Empty:
                return
            self.serie.write(texto.encode("ascii"))

    @pyqtSlot()
    def alternar_pausa(self):
        self.pausado = not self.pausado

    def baudios(self):
        """Baudio actual del puerto abierto."""
        return self._baudios

    def mandar(self, texto):
        """Encola un comando listo para el puerto."""
        self.cola_tx.put(texto)

    def pedir_baudios(self, baudios):
        self._baudios_pendientes = baudios

    def cerrar(self):
        if self.serie is not None and self.serie.is_open:
            self.serie.close()

    def repartir(self, cuerpo):
        if cuerpo.startswith("ACELEROMETRO,"):
            try:
                _, eje, valor = cuerpo.split(",")
            except ValueError:
                return  # campo raro con checksum: descarte
            if not self.pausado:
                self.dato_eje.emit(eje, float(valor))
        elif cuerpo.startswith("AMBIENTE,"):
            try:
                _, temp, hr = cuerpo.split(",")
            except ValueError:
                return
            self.ambiente.emit(float(temp), int(hr))
        elif cuerpo.startswith("OK,") or cuerpo.startswith("ERROR,"):
            self.respuesta.emit(cuerpo)
