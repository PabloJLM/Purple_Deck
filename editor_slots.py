#!/usr/bin/env python3
"""
editor_slots.py - Edita los 3 espacios libres del menu principal de Purple Deck.

Habla con el deck por USB (puerto serie) usando los mismos comandos que se
pueden escribir a mano en la terminal del deck ('slot help'). No hace falta
recompilar ni reflashear: lo que guardas aqui queda en la memoria del deck.

Uso:     python editor_slots.py
Requiere: pip install pyserial PyQt5

Importante: el puerto serie solo lo puede tener abierto un programa. Cierra el
monitor serie de PlatformIO / Arduino antes de conectar.
"""
import json
import sys
import time

import serial
from serial.tools import list_ports
from PyQt5 import QtCore, QtGui, QtWidgets

MIME = "application/x-purpledeck"
TIPOS = ["accion", "teclas", "texto", "comando"]
TIPO_AYUDA = {
    "accion":  "Una accion que el deck ya sabe hacer (volumen, captura, Git...).",
    "teclas":  "Una combinacion de teclas, por ejemplo  ctrl+shift+p  o  alt+left.",
    "texto":   "Escribe un texto en la PC.",
    "comando": "Escribe un texto en la PC y presiona Enter (para terminales).",
}
LEN_ETIQUETA, LEN_VALOR, LEN_ICONO = 23, 95, 31

# Lo que ya hay en la pagina 2 del menu (el firmware lo define; aqui es solo
# para dibujar el mapa). Los 3 espacios van despues de estos.
FIJOS_PAG2 = ["Pomodoro", "Reloj", "Bloquear", "Fotos"]


# ---------------------------------------------------------------------------
#  Comunicacion con el deck
# ---------------------------------------------------------------------------
class DeckError(Exception):
    pass


class Deck:
    """Linea de comandos del deck sobre el puerto serie."""

    def __init__(self, abrir=None):
        self._abrir = abrir or self._abrir_real
        self.ser = None

    @staticmethod
    def _abrir_real(puerto):
        s = serial.Serial()
        s.port = puerto
        s.baudrate = 115200
        s.timeout = 0.2
        # Con el USB nativo del ESP32-S3, DTR/RTS pueden reiniciar la placa.
        # Se dejan apagados ANTES de abrir.
        s.dtr = False
        s.rts = False
        s.open()
        return s

    @property
    def conectado(self):
        return self.ser is not None

    def conectar(self, puerto):
        self.cerrar()
        try:
            self.ser = self._abrir(puerto)
        except (serial.SerialException, OSError) as e:
            self.ser = None
            raise DeckError(f"No se pudo abrir {puerto}: {e}\n"
                            "¿Esta abierto el monitor serie en otro programa?")
        time.sleep(0.1)
        self.ser.reset_input_buffer()

    def cerrar(self):
        if self.ser is not None:
            try:
                self.ser.close()
            except Exception:
                pass
        self.ser = None

    def comando(self, linea, espera=3.0):
        """Manda un comando y devuelve las lineas de datos (SLOT / ACT)."""
        if self.ser is None:
            raise DeckError("No hay conexion con el deck.")
        try:
            self.ser.reset_input_buffer()
            self.ser.write((linea + "\n").encode("ascii", "replace"))
            datos = []
            fin = time.time() + espera
            resto = b""
            while time.time() < fin:
                trozo = self.ser.readline()
                if not trozo:
                    continue
                resto = trozo.rstrip(b"\r\n")
                txt = resto.decode("ascii", "replace").strip()
                if txt == "OK":
                    return datos
                if txt.startswith("ERR"):
                    raise DeckError(txt[3:].strip() or "error")
                if txt.startswith("SLOT ") or txt.startswith("ACT "):
                    datos.append(txt)
        except (serial.SerialException, OSError) as e:
            self.cerrar()
            raise DeckError(f"Se perdio la conexion: {e}")
        raise DeckError("El deck no respondio. ¿Esta cerrado el monitor serie?")

    # --- comandos de alto nivel ------------------------------------------
    def leer_slots(self):
        slots = [vacio() for _ in range(3)]
        for l in self.comando("slot list"):
            num, resto = l[5:].split("|", 1)
            tipo, etiq, valor, icono = (resto.split("|") + ["", "", "", ""])[:4]
            slots[int(num) - 1] = {"tipo": tipo, "etiqueta": etiq,
                                   "valor": valor, "icono": icono}
        return slots

    def leer_acciones(self):
        acc = []
        for l in self.comando("slot actions", espera=6.0):
            clave, grupo, nombre, atajo = (l[4:].split("|") + ["", "", "", ""])[:4]
            acc.append({"clave": clave, "grupo": grupo, "nombre": nombre, "atajo": atajo})
        return acc

    def guardar_slot(self, n, s):
        if s["tipo"] == "vacio":
            self.comando(f"slot clear {n}")
            return True
        icono = f' "{s["icono"]}"' if s["icono"] else ""
        self.comando(f'slot set {n} {s["tipo"]} "{s["etiqueta"]}" "{s["valor"]}"{icono}')
        return True

    def probar(self, n):
        self.comando(f"slot test {n}")
        return True


def vacio():
    return {"tipo": "vacio", "etiqueta": "", "valor": "", "icono": ""}


def revisar_slot(s):
    """Devuelve un texto de error, o None si el slot se puede mandar."""
    if s["tipo"] == "vacio":
        return None
    if not s["etiqueta"].strip():
        return "Falta la etiqueta."
    if not s["valor"].strip():
        return "Falta el valor."
    for nombre, v in (("etiqueta", s["etiqueta"]), ("valor", s["valor"]), ("icono", s["icono"])):
        if "|" in v or '"' in v:
            return f"La {nombre} no puede llevar | ni comillas."
        if not v.isascii() or any(ord(c) < 32 for c in v):
            return f"La {nombre} solo admite caracteres ASCII (sin acentos ni ñ)."
    if len(s["etiqueta"]) > LEN_ETIQUETA:
        return f"La etiqueta admite maximo {LEN_ETIQUETA} caracteres."
    if len(s["valor"]) > LEN_VALOR:
        return f"El valor admite maximo {LEN_VALOR} caracteres."
    if len(s["icono"]) > LEN_ICONO:
        return f"El icono admite maximo {LEN_ICONO} caracteres."
    if s["icono"] and not s["icono"].startswith("/"):
        return "El icono es una ruta que empieza con /  (por ejemplo /mi_icono.png)."
    return None


# ---------------------------------------------------------------------------
#  Interfaz
# ---------------------------------------------------------------------------
ESTILO = """
QWidget { background:#1b1530; color:#ece6ff; font-size:13px; }
QLineEdit, QComboBox, QPlainTextEdit, QListWidget {
    background:#261d45; border:1px solid #4a3a85; border-radius:6px; padding:4px; }
QPushButton { background:#5b3fb5; border:none; border-radius:6px; padding:7px 14px; }
QPushButton:hover { background:#7050d8; }
QPushButton:disabled { background:#3a3060; color:#8a80b0; }
QListWidget::item { padding:5px; }
QListWidget::item:selected { background:#5b3fb5; }
QLabel#titulo { font-size:15px; font-weight:bold; color:#e7c75f; }
QLabel#ayuda  { color:#a79bd6; }
"""


class Paleta(QtWidgets.QListWidget):
    """Lista de acciones que se arrastran a un espacio."""

    def __init__(self):
        super().__init__()
        self.setDragEnabled(True)
        self.setDragDropMode(QtWidgets.QAbstractItemView.DragOnly)

    def mimeData(self, items):
        m = QtCore.QMimeData()
        m.setData(MIME, json.dumps(items[0].data(QtCore.Qt.UserRole)).encode())
        return m


class Celda(QtWidgets.QFrame):
    """Una casilla del mapa de la pagina. Las fijas son solo de lectura."""

    def __init__(self, texto, fija=True):
        super().__init__()
        self.setMinimumSize(150, 84)
        self.lay = QtWidgets.QVBoxLayout(self)
        self.lay.setContentsMargins(8, 6, 8, 6)
        self.principal = QtWidgets.QLabel(texto)
        self.principal.setAlignment(QtCore.Qt.AlignCenter)
        self.principal.setWordWrap(True)
        self.secundario = QtWidgets.QLabel("")
        self.secundario.setAlignment(QtCore.Qt.AlignCenter)
        self.secundario.setObjectName("ayuda")
        self.secundario.setWordWrap(True)
        self.lay.addWidget(self.principal)
        self.lay.addWidget(self.secundario)
        self.setFija()

    def setFija(self):
        self.setStyleSheet("Celda { background:#2a2350; border:1px solid #3d3470; border-radius:8px; }"
                           "QLabel { background:transparent; color:#8f85bd; }")


class CeldaSlot(Celda):
    def __init__(self, idx, ventana):
        super().__init__("(vacio)", fija=False)
        self.idx = idx
        self.ventana = ventana
        self.setAcceptDrops(True)
        self._inicio = None
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._menu)
        self.pintar(vacio(), False)

    def pintar(self, s, sel):
        borde = "#e7c75f" if sel else "#7050d8"
        self.setStyleSheet(f"CeldaSlot {{ background:#3b2a78; border:2px dashed {borde}; border-radius:8px; }}"
                           "QLabel { background:transparent; }")
        if s["tipo"] == "vacio":
            self.principal.setText(f"Espacio {self.idx + 1}\n(arrastra algo aqui)")
            self.principal.setStyleSheet("color:#a79bd6;")
            self.secundario.setText("")
        else:
            self.principal.setText(s["etiqueta"].replace("\\n", "\n"))
            self.principal.setStyleSheet("color:#ffffff; font-weight:bold;")
            self.secundario.setText(f'{s["tipo"]}: {s["valor"]}')

    # arrastrar a otro espacio (intercambia)
    def mousePressEvent(self, e):
        self._inicio = e.pos()
        self.ventana.seleccionar(self.idx)

    def mouseMoveEvent(self, e):
        if self._inicio is None or not (e.buttons() & QtCore.Qt.LeftButton):
            return
        if (e.pos() - self._inicio).manhattanLength() < QtWidgets.QApplication.startDragDistance():
            return
        if self.ventana.slots[self.idx]["tipo"] == "vacio":
            return
        m = QtCore.QMimeData()
        m.setData(MIME, json.dumps({"kind": "slot", "idx": self.idx}).encode())
        d = QtGui.QDrag(self)
        d.setMimeData(m)
        d.exec_(QtCore.Qt.MoveAction)
        self._inicio = None

    def mouseDoubleClickEvent(self, e):
        self.ventana.editar(self.idx)

    def dragEnterEvent(self, e):
        if e.mimeData().hasFormat(MIME):
            e.acceptProposedAction()

    def dropEvent(self, e):
        datos = json.loads(bytes(e.mimeData().data(MIME)).decode())
        e.acceptProposedAction()
        self.ventana.soltar(self.idx, datos)

    def _menu(self, pos):
        m = QtWidgets.QMenu(self)
        a_edit = m.addAction("Editar...")
        a_vaciar = m.addAction("Vaciar")
        a_prob = m.addAction("Probar en el deck")
        r = m.exec_(self.mapToGlobal(pos))
        if r == a_edit:
            self.ventana.editar(self.idx)
        elif r == a_vaciar:
            self.ventana.vaciar(self.idx)
        elif r == a_prob:
            self.ventana.probar(self.idx)


class DialogoSlot(QtWidgets.QDialog):
    def __init__(self, parent, slot, acciones, tipo_inicial=None):
        super().__init__(parent)
        self.setWindowTitle("Editar espacio")
        self.setMinimumWidth(460)
        self.acciones = acciones
        f = QtWidgets.QFormLayout(self)

        self.tipo = QtWidgets.QComboBox()
        self.tipo.addItems(TIPOS)
        t = slot["tipo"] if slot["tipo"] in TIPOS else (tipo_inicial or "accion")
        self.tipo.setCurrentText(t)
        self.ayuda = QtWidgets.QLabel()
        self.ayuda.setObjectName("ayuda")
        self.ayuda.setWordWrap(True)

        self.etiqueta = QtWidgets.QLineEdit(slot["etiqueta"])
        self.etiqueta.setMaxLength(LEN_ETIQUETA)
        self.etiqueta.setPlaceholderText(r"texto del boton;  \n parte en dos lineas")

        self.valor = QtWidgets.QLineEdit(slot["valor"])
        self.valor.setMaxLength(LEN_VALOR)
        self.combo = QtWidgets.QComboBox()
        for a in acciones:
            self.combo.addItem(f'{a["grupo"]} / {a["nombre"]}', a)
        self._elegir_combo(slot["valor"])

        self.icono = QtWidgets.QLineEdit(slot["icono"])
        self.icono.setMaxLength(LEN_ICONO)
        self.icono.setPlaceholderText("opcional, ruta en la placa: /mi_icono.png")

        f.addRow("Tipo", self.tipo)
        f.addRow("", self.ayuda)
        f.addRow("Etiqueta", self.etiqueta)
        f.addRow("Valor", self.valor)
        f.addRow("Accion", self.combo)
        f.addRow("Icono", self.icono)
        self.form = f

        bb = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok | QtWidgets.QDialogButtonBox.Cancel)
        bb.accepted.connect(self._aceptar)
        bb.rejected.connect(self.reject)
        f.addRow(bb)

        self.tipo.currentTextChanged.connect(self._cambio_tipo)
        self._cambio_tipo(self.tipo.currentText())

    def _elegir_combo(self, clave):
        for i in range(self.combo.count()):
            if self.combo.itemData(i)["clave"] == clave:
                self.combo.setCurrentIndex(i)

    def _cambio_tipo(self, t):
        self.ayuda.setText(TIPO_AYUDA[t])
        es_accion = (t == "accion")
        for w, ver in ((self.valor, not es_accion), (self.combo, es_accion)):
            w.setVisible(ver)
            etiqueta = self.form.labelForField(w)
            if etiqueta is not None:
                etiqueta.setVisible(ver)
        if es_accion and not self.etiqueta.text() and self.combo.count():
            self.etiqueta.setText(self.combo.currentData()["nombre"][:LEN_ETIQUETA])

    def resultado(self):
        t = self.tipo.currentText()
        if t == "accion":
            a = self.combo.currentData()
            valor = a["clave"] if a else ""
        else:
            valor = self.valor.text()
        return {"tipo": t, "etiqueta": self.etiqueta.text().strip(),
                "valor": valor.strip(), "icono": self.icono.text().strip()}

    def _aceptar(self):
        err = revisar_slot(self.resultado())
        if err:
            QtWidgets.QMessageBox.warning(self, "Revisa los datos", err)
            return
        self.accept()


class Ventana(QtWidgets.QMainWindow):
    def __init__(self, deck=None):
        super().__init__()
        self.deck = deck or Deck()
        self.cfg = QtCore.QSettings("TeslaLab", "PurpleDeckSlots")
        self.slots = [vacio() for _ in range(3)]
        self.acciones = []
        self.sel = 0
        self.sucio = False
        self.setWindowTitle("Purple Deck - espacios del menu")
        self._ui()
        self._cargar_cache()
        self._refrescar_puertos()
        self._pintar()

    # --- construccion ---------------------------------------------------
    def _ui(self):
        c = QtWidgets.QWidget()
        self.setCentralWidget(c)
        raiz = QtWidgets.QVBoxLayout(c)

        # conexion
        barra = QtWidgets.QHBoxLayout()
        self.puertos = QtWidgets.QComboBox()
        self.puertos.setMinimumWidth(260)
        b_ref = QtWidgets.QPushButton("Actualizar")
        b_ref.clicked.connect(self._refrescar_puertos)
        self.b_con = QtWidgets.QPushButton("Conectar")
        self.b_con.clicked.connect(self.alternar_conexion)
        barra.addWidget(QtWidgets.QLabel("Puerto:"))
        barra.addWidget(self.puertos)
        barra.addWidget(b_ref)
        barra.addWidget(self.b_con)
        barra.addStretch(1)
        self.estado = QtWidgets.QLabel("Sin conexion")
        self.estado.setObjectName("ayuda")
        barra.addWidget(self.estado)
        raiz.addLayout(barra)

        cuerpo = QtWidgets.QHBoxLayout()
        raiz.addLayout(cuerpo, 1)

        # paleta
        izq = QtWidgets.QVBoxLayout()
        t = QtWidgets.QLabel("Acciones")
        t.setObjectName("titulo")
        izq.addWidget(t)
        h = QtWidgets.QLabel("Arrastra al mapa de la derecha")
        h.setObjectName("ayuda")
        izq.addWidget(h)
        self.paleta = Paleta()
        self.paleta.setMinimumWidth(250)
        izq.addWidget(self.paleta, 1)
        cuerpo.addLayout(izq)

        # mapa
        der = QtWidgets.QVBoxLayout()
        t2 = QtWidgets.QLabel("Menu principal - pagina 2")
        t2.setObjectName("titulo")
        der.addWidget(t2)
        h2 = QtWidgets.QLabel("Doble clic edita, clic derecho vacia, arrastra un espacio sobre otro para intercambiar.")
        h2.setObjectName("ayuda")
        h2.setWordWrap(True)
        der.addWidget(h2)
        grid = QtWidgets.QGridLayout()
        self.celdas = []
        orden = [("f", 0), ("f", 1), ("f", 2),
                 ("f", 3), ("s", 0), ("s", 1),
                 ("n", "< Ant"), ("s", 2), ("n", "Sig >")]
        for pos, (k, v) in enumerate(orden):
            if k == "f":
                w = Celda(FIJOS_PAG2[v])
            elif k == "n":
                w = Celda(v)
            else:
                w = CeldaSlot(v, self)
                self.celdas.append(w)
            grid.addWidget(w, pos // 3, pos % 3)
        der.addLayout(grid)
        der.addStretch(1)

        acc = QtWidgets.QHBoxLayout()
        self.b_leer = QtWidgets.QPushButton("Leer del deck")
        self.b_leer.clicked.connect(self.leer)
        self.b_guardar = QtWidgets.QPushButton("Guardar en el deck")
        self.b_guardar.clicked.connect(self.guardar)
        self.b_prob = QtWidgets.QPushButton("Probar el seleccionado")
        self.b_prob.clicked.connect(lambda: self.probar(self.sel))
        acc.addWidget(self.b_leer)
        acc.addWidget(self.b_guardar)
        acc.addWidget(self.b_prob)
        der.addLayout(acc)
        cuerpo.addLayout(der, 1)

        # registro
        self.log = QtWidgets.QPlainTextEdit()
        self.log.setReadOnly(True)
        self.log.setMaximumHeight(110)
        raiz.addWidget(self.log)
        self._habilitar()

    # --- estado ---------------------------------------------------------
    def _log(self, txt):
        self.log.appendPlainText(txt)

    def _habilitar(self):
        con = self.deck.conectado
        self.b_leer.setEnabled(con)
        self.b_guardar.setEnabled(con)
        self.b_prob.setEnabled(con)
        self.b_con.setText("Desconectar" if con else "Conectar")
        self.puertos.setEnabled(not con)

    def _titulo(self):
        self.setWindowTitle("Purple Deck - espacios del menu" + ("  *" if self.sucio else ""))

    def _pintar(self):
        for i, c in enumerate(self.celdas):
            c.pintar(self.slots[i], i == self.sel)
        self._titulo()

    def _llenar_paleta(self):
        self.paleta.clear()
        for texto, tipo in (("Combinacion de teclas...", "teclas"),
                            ("Texto...", "texto"),
                            ("Comando (texto + Enter)...", "comando")):
            it = QtWidgets.QListWidgetItem(texto)
            it.setData(QtCore.Qt.UserRole, {"kind": "plantilla", "tipo": tipo})
            f = it.font()
            f.setBold(True)
            it.setFont(f)
            self.paleta.addItem(it)
        grupo = None
        for a in self.acciones:
            if a["grupo"] != grupo:
                grupo = a["grupo"]
                cab = QtWidgets.QListWidgetItem(f"--- {grupo} ---")
                cab.setFlags(QtCore.Qt.NoItemFlags)
                self.paleta.addItem(cab)
            txt = a["nombre"] + (f'   ({a["atajo"]})' if a["atajo"] else "")
            it = QtWidgets.QListWidgetItem(txt)
            it.setData(QtCore.Qt.UserRole, dict(a, kind="accion"))
            self.paleta.addItem(it)

    def _cargar_cache(self):
        try:
            self.acciones = json.loads(self.cfg.value("acciones", "[]"))
        except (ValueError, TypeError):
            self.acciones = []
        self._llenar_paleta()

    # --- conexion ---------------------------------------------------------
    def _refrescar_puertos(self):
        actual = self.puertos.currentData() or self.cfg.value("puerto", "")
        self.puertos.clear()
        for p in sorted(list_ports.comports(), key=lambda p: p.device):
            self.puertos.addItem(f"{p.device}  -  {p.description}", p.device)
        i = self.puertos.findData(actual)
        if i >= 0:
            self.puertos.setCurrentIndex(i)

    def alternar_conexion(self):
        if self.deck.conectado:
            self.deck.cerrar()
            self.estado.setText("Sin conexion")
            self._habilitar()
            return
        puerto = self.puertos.currentData()
        if not puerto:
            QtWidgets.QMessageBox.information(self, "Puerto", "No hay ningun puerto serie. Conecta el deck por USB.")
            return
        self.conectar(puerto)

    def conectar(self, puerto):
        QtWidgets.QApplication.setOverrideCursor(QtCore.Qt.WaitCursor)
        try:
            self.deck.conectar(puerto)
            self.slots = self.deck.leer_slots()
            self.acciones = self.deck.leer_acciones()
            self.cfg.setValue("puerto", puerto)
            self.cfg.setValue("acciones", json.dumps(self.acciones))
            self.sucio = False
            self.estado.setText(f"Conectado a {puerto}")
            self._log(f"Conectado a {puerto}: {len(self.acciones)} acciones disponibles")
            self._llenar_paleta()
        except DeckError as e:
            self.deck.cerrar()
            self.estado.setText("Sin conexion")
            QtWidgets.QMessageBox.warning(self, "No se pudo conectar", str(e))
        finally:
            QtWidgets.QApplication.restoreOverrideCursor()
        self._habilitar()
        self._pintar()

    def _usar(self, fn, *args):
        """Ejecuta una operacion del deck mostrando los errores."""
        try:
            return fn(*args)
        except DeckError as e:
            QtWidgets.QMessageBox.warning(self, "Deck", str(e))
            self._habilitar()
            self.estado.setText("Sin conexion" if not self.deck.conectado else self.estado.text())
            return None

    # --- acciones de los botones -----------------------------------------
    def leer(self):
        r = self._usar(self.deck.leer_slots)
        if r is None:
            return
        self.slots = r
        self.sucio = False
        self._log("Leido del deck")
        self._pintar()

    def guardar(self):
        for i, s in enumerate(self.slots):
            e = revisar_slot(s)
            if e:
                QtWidgets.QMessageBox.warning(self, f"Espacio {i + 1}", e)
                return
        QtWidgets.QApplication.setOverrideCursor(QtCore.Qt.WaitCursor)
        try:
            for i, s in enumerate(self.slots):
                if not self._usar(self.deck.guardar_slot, i + 1, s):
                    return
                self._log(f"Espacio {i + 1}: {'vaciado' if s['tipo'] == 'vacio' else s['tipo'] + ' ' + s['etiqueta']}")
        finally:
            QtWidgets.QApplication.restoreOverrideCursor()
        self.sucio = False
        self._log("Guardado en el deck")
        self._titulo()

    def probar(self, idx):
        if self.slots[idx]["tipo"] == "vacio":
            return
        if self.sucio:
            QtWidgets.QMessageBox.information(self, "Probar", "Guarda primero: el deck prueba lo que tiene guardado.")
            return
        self._usar(self.deck.probar, idx + 1)

    # --- edicion del mapa ---------------------------------------------------
    def seleccionar(self, idx):
        self.sel = idx
        self._pintar()

    def _cambiar(self, idx, slot):
        self.slots[idx] = slot
        self.sucio = True
        self.sel = idx
        self._pintar()

    def editar(self, idx, tipo_inicial=None):
        d = DialogoSlot(self, self.slots[idx], self.acciones, tipo_inicial)
        if d.exec_() == QtWidgets.QDialog.Accepted:
            self._cambiar(idx, d.resultado())

    def vaciar(self, idx):
        self._cambiar(idx, vacio())

    def soltar(self, idx, datos):
        k = datos.get("kind")
        if k == "accion":
            self._cambiar(idx, {"tipo": "accion", "etiqueta": datos["nombre"][:LEN_ETIQUETA],
                                "valor": datos["clave"], "icono": ""})
        elif k == "plantilla":
            self.editar(idx, datos["tipo"])
        elif k == "slot":
            o = datos["idx"]
            if o != idx:
                self.slots[idx], self.slots[o] = self.slots[o], self.slots[idx]
                self.sucio = True
                self.sel = idx
                self._pintar()

    def closeEvent(self, e):
        if self.sucio:
            r = QtWidgets.QMessageBox.question(self, "Cambios sin guardar",
                                               "Hay cambios que no se han guardado en el deck. ¿Salir igual?")
            if r != QtWidgets.QMessageBox.Yes:
                e.ignore()
                return
        self.deck.cerrar()
        e.accept()


def main():
    app = QtWidgets.QApplication(sys.argv)
    app.setStyleSheet(ESTILO)
    w = Ventana()
    w.resize(900, 640)
    w.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
