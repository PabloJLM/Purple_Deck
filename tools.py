#!/usr/bin/env python3
"""
tools.py - Herramientas de Purple Deck en un solo programa.

  Menus    : reorganiza todos los menus del deck (arrastrar y soltar) y edita las
             macros de los 3 menus Custom (9 cada uno; las teclas se capturan)
  Red      : WiFi y FTP del deck, sin recompilar ni reflashear
  Temas    : generador de temas de color (exporta C++ RGB565, JSON, CSS...)
  Fotos    : convierte imagenes a JPG/PNG para el visor de fotos del deck

Espacios y Red hablan con el deck por USB (puerto serie) con los mismos
comandos que se pueden escribir a mano en su terminal ('slot help', 'red help').
Lo que se guarda queda en la memoria del deck (NVS): no hay que reflashear.

Uso:      python tools.py
Requiere: pip install pyserial PyQt5 pillow

Importante: el puerto serie solo lo puede tener abierto un programa. Cierra el
monitor serie de PlatformIO / Arduino antes de conectar.
"""
import colorsys
import json
import os
import sys
import time

import serial
from serial.tools import list_ports
from PyQt5 import QtCore, QtGui, QtWidgets

TIPOS = ["accion", "teclas", "texto", "comando"]
TIPO_AYUDA = {
    "accion":  "Una accion que el deck ya sabe hacer (volumen, captura, Git...).",
    "teclas":  "Una combinacion de teclas, por ejemplo  ctrl+shift+p  o  alt+left.",
    "texto":   "Escribe un texto en la PC.",
    "comando": "Escribe un texto en la PC y presiona Enter (para terminales).",
}
LEN_ETIQUETA, LEN_VALOR, LEN_ICONO = 23, 95, 31

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
        """Manda un comando y devuelve las lineas de datos (SLOT / ACT / RED)."""
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
                if txt.startswith(("SLOT ", "ACT ", "RED ", "MENU ", "ITEM ")):
                    datos.append(txt)
        except (serial.SerialException, OSError) as e:
            self.cerrar()
            raise DeckError(f"Se perdio la conexion: {e}")
        raise DeckError("El deck no respondio. ¿Esta cerrado el monitor serie?")

    # --- comandos de alto nivel ------------------------------------------
    def leer_slots(self):
        slots = [vacio() for _ in range(27)]
        for l in self.comando("slot list", espera=6.0):
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

    def leer_red(self):
        """Devuelve {'ssid','pass','ftp_user','ftp_pass'}."""
        r = {"ssid": "", "pass": "", "ftp_user": "", "ftp_pass": ""}
        for l in self.comando("red get"):
            tipo, a, b = (l[4:].split("|") + ["", ""])[:3]
            if tipo == "wifi":
                r["ssid"], r["pass"] = a, b
            elif tipo == "ftp":
                r["ftp_user"], r["ftp_pass"] = a, b
        return r

    def leer_menus(self):
        """Devuelve (menus, items). menus: id, clave, nombre, custom. items: clave, menu (-1 oculto), etiqueta, defecto."""
        menus, items = [], []
        for l in self.comando("menu list", espera=8.0):
            if l.startswith("MENU "):
                i, clave, nombre, custom, mx = (l[5:].split("|") + ["", "", "", "0", "28"])[:5]
                menus.append({"id": int(i), "clave": clave, "nombre": nombre,
                              "custom": custom == "1", "max": int(mx or 28)})
            elif l.startswith("ITEM "):
                clave, menu, etiq, defecto = (l[5:].split("|") + ["", "-1", "", "0"])[:4]
                items.append({"clave": clave, "menu": int(menu),
                              "etiqueta": etiq.replace("\\n", "\n"), "defecto": int(defecto or 0)})
        return menus, items

    def guardar_layout(self, listas, ocultos, nombres):
        """listas: {id_menu: [claves]}, ocultos: [claves], nombres: {k(1-3): texto}."""
        for k, txt in nombres.items():
            self.comando(f'menu name {k} "{txt}"')
        self.comando("menu begin")
        for m in sorted(listas):
            for clave in listas[m]:
                self.comando(f"menu add {m} {clave}")
        for clave in ocultos:
            self.comando(f"menu hide {clave}")
        self.comando("menu commit", espera=6.0)

    def restablecer_menus(self):
        self.comando("menu reset", espera=6.0)

    def guardar_wifi(self, ssid, clave):
        self.comando(f'red wifi "{ssid}" "{clave}"')

    def guardar_ftp(self, usuario, clave):
        self.comando(f'red ftp "{usuario}" "{clave}"')


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
QWidget { background:#202020; color:#e0e0e0; font-size:13px; }
QLineEdit, QComboBox, QPlainTextEdit, QListWidget, QDoubleSpinBox {
    background:#2b2b2b; border:1px solid #3c3c3c; padding:4px; }
QPushButton { background:#363636; border:1px solid #444; padding:6px 14px; }
QPushButton:hover { background:#444; }
QPushButton:disabled { color:#777; background:#2b2b2b; }
QListWidget::item:selected { background:#3d6fb4; }
QTabWidget::pane { border:1px solid #3c3c3c; }
QTabBar::tab { background:#2b2b2b; padding:6px 16px; border:1px solid #3c3c3c; }
QTabBar::tab:selected { background:#3d6fb4; }
QLabel#titulo { font-size:15px; font-weight:bold; }
QLabel#ayuda  { color:#999; }
"""


NOMBRES_TECLA = {
    QtCore.Qt.Key_Return: "enter", QtCore.Qt.Key_Enter: "enter", QtCore.Qt.Key_Escape: "esc",
    QtCore.Qt.Key_Backspace: "backspace", QtCore.Qt.Key_Tab: "tab", QtCore.Qt.Key_Backtab: "tab",
    QtCore.Qt.Key_Space: "space", QtCore.Qt.Key_Insert: "insert", QtCore.Qt.Key_Delete: "delete",
    QtCore.Qt.Key_Home: "home", QtCore.Qt.Key_End: "end", QtCore.Qt.Key_PageUp: "pageup",
    QtCore.Qt.Key_PageDown: "pagedown", QtCore.Qt.Key_Left: "left", QtCore.Qt.Key_Right: "right",
    QtCore.Qt.Key_Up: "up", QtCore.Qt.Key_Down: "down",
}
for _i in range(1, 13):
    NOMBRES_TECLA[getattr(QtCore.Qt, f"Key_F{_i}")] = f"f{_i}"


class DialogoSlot(QtWidgets.QDialog):
    def __init__(self, parent, slot, acciones, tipo_inicial=None):
        super().__init__(parent)
        self.setWindowTitle("Editar macro")
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
        self.b_cap = QtWidgets.QPushButton("Capturar teclas (5 s)")
        self.b_cap.clicked.connect(self._capturar)
        f.addRow("", self.b_cap)
        f.addRow("Accion", self.combo)
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
        self.b_cap.setVisible(t == "teclas")
        es_accion = (t == "accion")
        for w, ver in ((self.valor, not es_accion), (self.combo, es_accion)):
            w.setVisible(ver)
            etiqueta = self.form.labelForField(w)
            if etiqueta is not None:
                etiqueta.setVisible(ver)
        if es_accion and not self.etiqueta.text() and self.combo.count():
            self.etiqueta.setText(self.combo.currentData()["nombre"][:LEN_ETIQUETA])

    # --- captura de teclas: el usuario presiona la combinacion en la PC ---
    def _capturar(self):
        self.capturando = True
        self.cap_mods = []
        self.restante = 5
        self.b_cap.setText("Presiona la combinacion...  5")
        self.grabKeyboard()
        self.reloj = QtCore.QTimer(self)
        self.reloj.timeout.connect(self._tic)
        self.reloj.start(1000)

    def _tic(self):
        self.restante -= 1
        if self.restante <= 0:
            self._fin_captura(None)
        else:
            self.b_cap.setText(f"Presiona la combinacion...  {self.restante}")

    def _fin_captura(self, combo):
        self.capturando = False
        self.reloj.stop()
        self.releaseKeyboard()
        self.b_cap.setText("Capturar teclas (5 s)")
        if combo:
            self.valor.setText(combo)
            if not self.etiqueta.text().strip():
                self.etiqueta.setText(combo.title().replace("+", "+")[:LEN_ETIQUETA])
        else:
            QtWidgets.QMessageBox.information(self, "Captura", "No se capturo ninguna combinacion.")

    def event(self, e):
        if getattr(self, "capturando", False):
            if e.type() == QtCore.QEvent.ShortcutOverride:
                e.accept()
                return True
            if e.type() == QtCore.QEvent.KeyPress:
                self._tecla(e)
                return True
        return super().event(e)

    def _tecla(self, e):
        if e.isAutoRepeat():
            return
        K = QtCore.Qt
        if e.key() in (K.Key_Control, K.Key_Shift, K.Key_Alt, K.Key_Meta, K.Key_AltGr):
            return  # solo modificadores: se espera la tecla principal
        nombre = NOMBRES_TECLA.get(e.key())
        if nombre is None and 0x20 <= e.key() <= 0x7E:
            nombre = chr(e.key()).lower()
        if nombre is None:
            return
        m = e.modifiers()
        partes = []
        if m & K.ControlModifier: partes.append("ctrl")
        if m & K.ShiftModifier:   partes.append("shift")
        if m & K.AltModifier:     partes.append("alt")
        if m & K.MetaModifier:    partes.append("win")
        partes.append(nombre)
        self._fin_captura("+".join(partes))

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



# ---------------------------------------------------------------------------
#  Pestana: menus (reorganizar botones, macros de los menus Custom)
# ---------------------------------------------------------------------------
ROL = QtCore.Qt.UserRole


class Arbol(QtWidgets.QTreeWidget):
    """Menus como raices y botones como hijos; arrastrar mueve u ordena."""
    cambiado = QtCore.pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setHeaderHidden(True)
        self.setIndentation(18)
        self.setDragEnabled(True)
        self.setAcceptDrops(True)
        self.setDragDropMode(QtWidgets.QAbstractItemView.InternalMove)
        self.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)

    def _orden(self, it):
        p = it.parent()
        return (self.indexOfTopLevelItem(p), p.indexOfChild(it))

    def dropEvent(self, e):
        destino = self.itemAt(e.pos())
        mover = sorted([i for i in self.selectedItems() if i.parent() is not None], key=self._orden)
        if destino is None or not mover or destino in mover:
            e.ignore()
            return
        if destino.parent() is None:                      # soltado sobre un menu: al final
            padre, idx = destino, destino.childCount()
        else:
            padre = destino.parent()
            idx = padre.indexOfChild(destino)
            if self.dropIndicatorPosition() != QtWidgets.QAbstractItemView.AboveItem:
                idx += 1
        for it in mover:
            p = it.parent()
            i = p.indexOfChild(it)
            p.takeChild(i)
            if p is padre and i < idx:
                idx -= 1
        for k, it in enumerate(mover):
            padre.insertChild(idx + k, it)
        padre.setExpanded(True)
        e.setDropAction(QtCore.Qt.IgnoreAction)
        e.accept()
        self.clearSelection()
        for it in mover:
            it.setSelected(True)
        self.cambiado.emit()


class PanelMenus(QtWidgets.QWidget):
    def __init__(self, deck, log):
        super().__init__()
        self.deck = deck
        self.log_fn = log
        self.cfg = QtCore.QSettings("TeslaLab", "PurpleDeckTools")
        self.slots = [vacio() for _ in range(27)]
        self.slots_orig = [vacio() for _ in range(27)]
        self.acciones = []
        self.menus = []
        self.nombres_orig = {}
        self.sucio = False
        self.al_cambiar_sucio = None
        self._ui()
        self._cargar_cache()

    # --- construccion ---
    def _ui(self):
        raiz = QtWidgets.QHBoxLayout(self)
        izq = QtWidgets.QVBoxLayout()
        t = QtWidgets.QLabel("Menus del deck")
        t.setObjectName("titulo")
        izq.addWidget(t)
        h = QtWidgets.QLabel("Arrastra botones para ordenarlos o pasarlos de un menu a otro. "
                             "Lo que pongas en 'Ocultos' no se muestra en el deck.")
        h.setObjectName("ayuda")
        h.setWordWrap(True)
        izq.addWidget(h)
        self.arbol = Arbol()
        self.arbol.cambiado.connect(self._marcar)
        self.arbol.itemSelectionChanged.connect(self._habilitar)
        self.arbol.itemDoubleClicked.connect(lambda *_: self.editar())
        izq.addWidget(self.arbol, 1)
        raiz.addLayout(izq, 2)

        der = QtWidgets.QVBoxLayout()
        self.info = QtWidgets.QLabel("")
        self.info.setObjectName("ayuda")
        self.info.setWordWrap(True)
        der.addWidget(self.info)
        self.b_nueva = QtWidgets.QPushButton("Nueva macro")
        self.b_nueva.clicked.connect(self.nueva)
        self.b_editar = QtWidgets.QPushButton("Editar macro")
        self.b_editar.clicked.connect(self.editar)
        self.b_prob = QtWidgets.QPushButton("Probar macro")
        self.b_prob.clicked.connect(self.probar)
        self.b_borrar = QtWidgets.QPushButton("Borrar macro")
        self.b_borrar.clicked.connect(self.borrar)
        self.b_ocultar = QtWidgets.QPushButton("Ocultar")
        self.b_ocultar.clicked.connect(self.ocultar)
        self.b_mostrar = QtWidgets.QPushButton("Mostrar (a su menu)")
        self.b_mostrar.clicked.connect(self.mostrar)
        self.b_renombrar = QtWidgets.QPushButton("Renombrar menu")
        self.b_renombrar.clicked.connect(self.renombrar)
        for b in (self.b_nueva, self.b_editar, self.b_prob, self.b_borrar,
                  self.b_ocultar, self.b_mostrar, self.b_renombrar):
            der.addWidget(b)
        der.addStretch(1)
        self.b_leer = QtWidgets.QPushButton("Leer del deck")
        self.b_leer.clicked.connect(self.leer)
        self.b_guardar = QtWidgets.QPushButton("Guardar en el deck")
        self.b_guardar.clicked.connect(self.guardar)
        self.b_reset = QtWidgets.QPushButton("Restablecer de fabrica")
        self.b_reset.clicked.connect(self.restablecer)
        for b in (self.b_leer, self.b_guardar, self.b_reset):
            der.addWidget(b)
        w = QtWidgets.QWidget()
        w.setLayout(der)
        w.setFixedWidth(210)
        raiz.addWidget(w)
        self._habilitar()

    def _cargar_cache(self):
        try:
            self.acciones = json.loads(self.cfg.value("acciones", "[]"))
        except (ValueError, TypeError):
            self.acciones = []

    # --- helpers de datos ---
    def _marcar(self, sucio=True):
        self.sucio = sucio
        if self.al_cambiar_sucio:
            self.al_cambiar_sucio(sucio)
        self._habilitar()

    @staticmethod
    def _slot_de(clave):
        """'c2/3' -> indice 0..26 de la macro, o None."""
        if clave[:1] == "c" and "/" in clave and clave[1:2] in "123":
            try:
                return (int(clave[1]) - 1) * 9 + int(clave.split("/")[1]) - 1
            except ValueError:
                return None
        return None

    def _texto(self, d):
        et = d["etiqueta"].replace("\n", " / ")
        n = d.get("slot")
        if n is not None:
            s = self.slots[n]
            return f'{et}    [{s["tipo"]}: {s["valor"]}]'
        return et

    def _crear_item(self, d):
        it = QtWidgets.QTreeWidgetItem([self._texto(d)])
        it.setData(0, ROL, d)
        it.setFlags(it.flags() & ~QtCore.Qt.ItemIsDropEnabled | QtCore.Qt.ItemIsDragEnabled)
        return it

    def _raiz_menu(self, mid):
        for i in range(self.arbol.topLevelItemCount()):
            r = self.arbol.topLevelItem(i)
            if r.data(0, ROL)["menu"] == mid:
                return r
        return None

    def _sel(self):
        return [i for i in self.arbol.selectedItems()]

    def _seleccion_item(self):
        s = [i for i in self._sel() if i.parent() is not None]
        return s[0] if len(s) == 1 else None

    def _menu_de(self, it):
        r = it if it.parent() is None else it.parent()
        return r.data(0, ROL)["menu"]

    def _habilitar(self):
        con = self.deck.conectado
        for b in (self.b_leer, self.b_guardar, self.b_reset):
            b.setEnabled(con)
        sel = self._sel()
        item = self._seleccion_item()
        d = item.data(0, ROL) if item else None
        en_custom = bool(sel) and self._menu_de(sel[0]) >= M_CUSTOM1_ID
        es_macro = d is not None and d.get("slot") is not None
        raiz_custom = len(sel) == 1 and sel[0].parent() is None and sel[0].data(0, ROL)["menu"] >= M_CUSTOM1_ID
        self.b_nueva.setEnabled(con and en_custom)
        self.b_editar.setEnabled(es_macro)
        self.b_prob.setEnabled(con and es_macro and not self.sucio)
        self.b_borrar.setEnabled(es_macro)
        hijos = [i for i in sel if i.parent() is not None]
        self.b_ocultar.setEnabled(bool(hijos) and all(self._menu_de(i) != -1 for i in hijos))
        self.b_mostrar.setEnabled(bool(hijos) and all(self._menu_de(i) == -1 for i in hijos))
        self.b_renombrar.setEnabled(raiz_custom)
        if d is not None:
            origen = self._nombre_menu(d["defecto"])
            self.info.setText(f'{d["etiqueta"].replace(chr(10), " / ")}\nclave: {d["clave"]}\nde fabrica en: {origen}')
        else:
            self.info.setText("")

    def _nombre_menu(self, mid):
        for m in self.menus:
            if m["id"] == mid:
                return m["nombre"]
        return "?"

    # --- conexion ---
    def al_conectar(self):
        self.leer(silencioso=True)

    def al_desconectar(self):
        self._habilitar()

    def leer(self, silencioso=False):
        if self.sucio and not silencioso:
            r = QtWidgets.QMessageBox.question(self, "Leer del deck", "Hay cambios sin guardar. ¿Descartarlos?")
            if r != QtWidgets.QMessageBox.Yes:
                return
        try:
            self.slots = self.deck.leer_slots()
            self.acciones = self.deck.leer_acciones()
            menus, items = self.deck.leer_menus()
        except DeckError as e:
            QtWidgets.QMessageBox.warning(self, "Deck", str(e))
            return
        self.cfg.setValue("acciones", json.dumps(self.acciones))
        self.slots_orig = [dict(s) for s in self.slots]
        self.menus = menus
        self.nombres_orig = {m["id"]: m["nombre"] for m in menus if m["custom"]}
        self.arbol.clear()
        for m in menus:
            r = QtWidgets.QTreeWidgetItem([m["nombre"]])
            r.setData(0, ROL, {"menu": m["id"], "nombre": m["nombre"], "custom": m["custom"], "max": m["max"]})
            r.setFlags((r.flags() | QtCore.Qt.ItemIsDropEnabled) & ~QtCore.Qt.ItemIsDragEnabled)
            f = r.font(0)
            f.setBold(True)
            r.setFont(0, f)
            self.arbol.addTopLevelItem(r)
        oc = QtWidgets.QTreeWidgetItem(["Ocultos"])
        oc.setData(0, ROL, {"menu": -1, "nombre": "Ocultos", "custom": False, "max": 999})
        oc.setFlags((oc.flags() | QtCore.Qt.ItemIsDropEnabled) & ~QtCore.Qt.ItemIsDragEnabled)
        self.arbol.addTopLevelItem(oc)
        for it in items:
            d = dict(it, slot=self._slot_de(it["clave"]))
            r = self._raiz_menu(it["menu"])
            if r is not None:
                r.addChild(self._crear_item(d))
        for i in range(self.arbol.topLevelItemCount()):
            r = self.arbol.topLevelItem(i)
            r.setText(0, f'{r.data(0, ROL)["nombre"]}  ({r.childCount()})')
            r.setExpanded(r.data(0, ROL)["menu"] != -1)
        self._marcar(False)
        self.log_fn(f"Menus leidos: {len(items)} botones, {len(self.acciones)} acciones")

    def _refrescar_cuentas(self):
        for i in range(self.arbol.topLevelItemCount()):
            r = self.arbol.topLevelItem(i)
            r.setText(0, f'{r.data(0, ROL)["nombre"]}  ({r.childCount()})')

    # --- operaciones ---
    def nueva(self):
        sel = self._sel()
        if not sel:
            return
        mid = self._menu_de(sel[0])
        k = mid - M_CUSTOM1_ID + 1
        usados = {self.arbol.topLevelItem(i).child(j).data(0, ROL)["clave"]
                  for i in range(self.arbol.topLevelItemCount())
                  for j in range(self.arbol.topLevelItem(i).childCount())}
        libre = None
        for j in range(1, 10):
            n = (k - 1) * 9 + j - 1
            if f"c{k}/{j}" not in usados and self.slots[n]["tipo"] == "vacio":
                libre = j
                break
        if libre is None:
            QtWidgets.QMessageBox.information(self, "Macro", "Este menu ya tiene sus 9 macros.")
            return
        n = (k - 1) * 9 + libre - 1
        dlg = DialogoSlot(self, vacio(), self.acciones, "teclas")
        if dlg.exec_() != QtWidgets.QDialog.Accepted:
            return
        self.slots[n] = dlg.resultado()
        d = {"clave": f"c{k}/{libre}", "menu": mid, "etiqueta": self.slots[n]["etiqueta"], "defecto": mid, "slot": n}
        r = self._raiz_menu(mid)
        r.addChild(self._crear_item(d))
        r.setExpanded(True)
        self._refrescar_cuentas()
        self._marcar()

    def editar(self):
        it = self._seleccion_item()
        if it is None:
            return
        d = it.data(0, ROL)
        n = d.get("slot")
        if n is None:
            return
        dlg = DialogoSlot(self, self.slots[n], self.acciones)
        if dlg.exec_() != QtWidgets.QDialog.Accepted:
            return
        self.slots[n] = dlg.resultado()
        d["etiqueta"] = self.slots[n]["etiqueta"]
        it.setData(0, ROL, d)
        it.setText(0, self._texto(d))
        self._marcar()

    def probar(self):
        it = self._seleccion_item()
        if it is None or it.data(0, ROL).get("slot") is None:
            return
        try:
            self.deck.probar(it.data(0, ROL)["slot"] + 1)
        except DeckError as e:
            QtWidgets.QMessageBox.warning(self, "Deck", str(e))

    def borrar(self):
        it = self._seleccion_item()
        if it is None:
            return
        d = it.data(0, ROL)
        if d.get("slot") is None:
            return
        self.slots[d["slot"]] = vacio()
        it.parent().removeChild(it)
        self._refrescar_cuentas()
        self._marcar()

    def _mover_a(self, items, raiz):
        for it in items:
            it.parent().removeChild(it)
            raiz.addChild(it)
        raiz.setExpanded(True)
        self._refrescar_cuentas()
        self._marcar()

    def ocultar(self):
        hijos = [i for i in self._sel() if i.parent() is not None]
        self._mover_a(hijos, self._raiz_menu(-1))

    def mostrar(self):
        for it in [i for i in self._sel() if i.parent() is not None]:
            d = it.data(0, ROL)
            r = self._raiz_menu(d["defecto"]) or self._raiz_menu(0)
            self._mover_a([it], r)

    def renombrar(self):
        sel = self._sel()
        if len(sel) != 1 or sel[0].parent() is not None:
            return
        d = sel[0].data(0, ROL)
        txt, ok = QtWidgets.QInputDialog.getText(self, "Renombrar menu", "Nombre (max 15, ASCII):", text=d["nombre"])
        if not ok:
            return
        txt = txt.strip()
        if not txt or len(txt) > 15 or not txt.isascii() or any(c in txt for c in '|,"'):
            QtWidgets.QMessageBox.warning(self, "Nombre", "Usa 1 a 15 caracteres ASCII, sin | , ni comillas.")
            return
        d["nombre"] = txt
        sel[0].setData(0, ROL, d)
        self._refrescar_cuentas()
        self._marcar()

    def restablecer(self):
        r = QtWidgets.QMessageBox.question(self, "Restablecer",
                                           "Vuelve todos los menus a como vienen de fabrica (no borra las macros). ¿Seguir?")
        if r != QtWidgets.QMessageBox.Yes:
            return
        try:
            self.deck.restablecer_menus()
        except DeckError as e:
            QtWidgets.QMessageBox.warning(self, "Deck", str(e))
            return
        self.sucio = False
        self.leer(silencioso=True)

    def guardar(self):
        for n, s in enumerate(self.slots):
            e = revisar_slot(s)
            if e:
                QtWidgets.QMessageBox.warning(self, f"Macro {n + 1}", e)
                return
        listas, ocultos, nombres = {}, [], {}
        for i in range(self.arbol.topLevelItemCount()):
            r = self.arbol.topLevelItem(i)
            rd = r.data(0, ROL)
            claves = [r.child(j).data(0, ROL)["clave"] for j in range(r.childCount())]
            if rd["menu"] == -1:
                ocultos = claves
                continue
            if len(claves) > rd["max"]:
                QtWidgets.QMessageBox.warning(self, "Menu lleno", f'{rd["nombre"]} admite maximo {rd["max"]} botones.')
                return
            listas[rd["menu"]] = claves
            if rd["custom"] and rd["nombre"] != self.nombres_orig.get(rd["menu"]):
                nombres[rd["menu"] - M_CUSTOM1_ID + 1] = rd["nombre"]
        QtWidgets.QApplication.setOverrideCursor(QtCore.Qt.WaitCursor)
        try:
            for n, s in enumerate(self.slots):
                if s != self.slots_orig[n]:
                    self.deck.guardar_slot(n + 1, s)
            self.deck.guardar_layout(listas, ocultos, nombres)
        except DeckError as e:
            QtWidgets.QMessageBox.warning(self, "Deck", str(e))
            return
        finally:
            QtWidgets.QApplication.restoreOverrideCursor()
        self.log_fn("Menus guardados en el deck")
        self.leer(silencioso=True)

    def puede_cerrar(self):
        if not self.sucio:
            return True
        r = QtWidgets.QMessageBox.question(self, "Cambios sin guardar",
                                           "Hay cambios de menus sin guardar en el deck. ¿Salir igual?")
        return r == QtWidgets.QMessageBox.Yes


M_CUSTOM1_ID = 8   # igual que MenuId::M_CUSTOM1 del firmware (Drivers/menus.h)


# ---------------------------------------------------------------------------
#  Pestana: red (WiFi y FTP)
# ---------------------------------------------------------------------------
def revisar_red(ssid, clave, fu, fp):
    for nombre, v in (("SSID", ssid), ("contrasena", clave), ("usuario FTP", fu), ("clave FTP", fp)):
        if '"' in v or not v.isascii() or any(ord(c) < 32 for c in v):
            return f"{nombre}: solo ASCII y sin comillas."
    if not ssid:
        return "El SSID no puede estar vacio."
    if len(ssid.encode()) > 32:
        return "El SSID admite maximo 32 caracteres."
    if clave and len(clave) < 8:
        return "Una contrasena WPA2 necesita 8 o mas caracteres."
    if len(clave) > 63:
        return "La contrasena admite maximo 63 caracteres."
    if not fu or not fp:
        return "El usuario y la clave del FTP no pueden estar vacios."
    if len(fu) > 31 or len(fp) > 31:
        return "Usuario y clave del FTP admiten maximo 31 caracteres."
    return None


class PanelRed(QtWidgets.QWidget):
    def __init__(self, deck, log):
        super().__init__()
        self.deck = deck
        self.log_fn = log
        raiz = QtWidgets.QVBoxLayout(self)
        t = QtWidgets.QLabel("WiFi y FTP del deck")
        t.setObjectName("titulo")
        raiz.addWidget(t)
        h = QtWidgets.QLabel("Se guardan en la memoria del deck: no hace falta recompilar ni reflashear. "
                             "El WiFi se reconecta al guardar; el FTP nuevo se usa la proxima vez que abras su pantalla.")
        h.setObjectName("ayuda")
        h.setWordWrap(True)
        raiz.addWidget(h)

        form = QtWidgets.QFormLayout()
        self.ssid = QtWidgets.QLineEdit()
        self.clave = QtWidgets.QLineEdit()
        self.clave.setEchoMode(QtWidgets.QLineEdit.Password)
        self.fu = QtWidgets.QLineEdit()
        self.fp = QtWidgets.QLineEdit()
        self.fp.setEchoMode(QtWidgets.QLineEdit.Password)
        form.addRow("Red WiFi (SSID)", self.ssid)
        form.addRow("Contrasena WiFi", self.clave)
        form.addRow("Usuario FTP", self.fu)
        form.addRow("Clave FTP", self.fp)
        raiz.addLayout(form)
        ver = QtWidgets.QCheckBox("Mostrar contrasenas")
        ver.toggled.connect(self._ver)
        raiz.addWidget(ver)

        fila = QtWidgets.QHBoxLayout()
        self.b_leer = QtWidgets.QPushButton("Leer del deck")
        self.b_leer.clicked.connect(self.leer)
        self.b_guardar = QtWidgets.QPushButton("Guardar en el deck")
        self.b_guardar.clicked.connect(self.guardar)
        fila.addWidget(self.b_leer)
        fila.addWidget(self.b_guardar)
        fila.addStretch(1)
        raiz.addLayout(fila)
        raiz.addStretch(1)
        self._habilitar()

    def _ver(self, on):
        modo = QtWidgets.QLineEdit.Normal if on else QtWidgets.QLineEdit.Password
        self.clave.setEchoMode(modo)
        self.fp.setEchoMode(modo)

    def _habilitar(self):
        con = self.deck.conectado
        self.b_leer.setEnabled(con)
        self.b_guardar.setEnabled(con)

    def al_conectar(self):
        self.leer(silencioso=True)
        self._habilitar()

    def al_desconectar(self):
        self._habilitar()

    def leer(self, silencioso=False):
        try:
            r = self.deck.leer_red()
        except DeckError as e:
            QtWidgets.QMessageBox.warning(self, "Deck", str(e))
            return
        self.ssid.setText(r["ssid"])
        self.clave.setText(r["pass"])
        self.fu.setText(r["ftp_user"])
        self.fp.setText(r["ftp_pass"])
        self.log_fn("Red leida del deck")

    def guardar(self):
        e = revisar_red(self.ssid.text(), self.clave.text(), self.fu.text(), self.fp.text())
        if e:
            QtWidgets.QMessageBox.warning(self, "Revisa los datos", e)
            return
        try:
            self.deck.guardar_wifi(self.ssid.text(), self.clave.text())
            self.deck.guardar_ftp(self.fu.text(), self.fp.text())
        except DeckError as ex:
            QtWidgets.QMessageBox.warning(self, "Deck", str(ex))
            return
        self.log_fn("Red guardada en el deck (WiFi reconectando)")


# ---------------------------------------------------------------------------
#  Temas: logica de color y exportadores
# ---------------------------------------------------------------------------


# ---------- utilidades de color ----------

def hex_a_rgb(h: str):
    h = h.lstrip("#")
    return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))


def rgb_a_hex(rgb):
    r, g, b = (max(0, min(255, int(round(c)))) for c in rgb)
    return f"#{r:02X}{g:02X}{b:02X}"


def rgb_a_hsl(rgb):
    r, g, b = [c / 255.0 for c in rgb]
    h, l, s = colorsys.rgb_to_hls(r, g, b)
    return h * 360.0, s * 100.0, l * 100.0


def hsl_a_rgb(h, s, l):
    h = (h % 360) / 360.0
    s = max(0, min(100, s)) / 100.0
    l = max(0, min(100, l)) / 100.0
    r, g, b = colorsys.hls_to_rgb(h, l, s)
    return (r * 255, g * 255, b * 255)


def rgb_a_rgb565(rgb):
    r, g, b = (max(0, min(255, int(round(c)))) for c in rgb)
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def a_565(rgb):
    """Redondea un color a lo que la pantalla puede mostrar (RGB565). El
    contraste se verifica sobre esto, no sobre el color de 24 bits: si no,
    un 4.52 puede quedar en 4.48 al convertirlo."""
    v = rgb_a_rgb565(rgb)
    r, g, b = (v >> 11) & 0x1F, (v >> 5) & 0x3F, v & 0x1F
    return (round(r * 255 / 31), round(g * 255 / 63), round(b * 255 / 31))


def mix(rgb1, rgb2, t):
    return tuple(a + (b - a) * t for a, b in zip(rgb1, rgb2))


def ajustar_l(rgb, delta):
    h, s, l = rgb_a_hsl(rgb)
    return hsl_a_rgb(h, s, max(0, min(100, l + delta)))


def ajustar_hsl(rgb, dh=0, ds=0, dl=0):
    h, s, l = rgb_a_hsl(rgb)
    return hsl_a_rgb(h + dh, max(0, min(100, s + ds)), max(0, min(100, l + dl)))


def luminancia(rgb):
    """Luminancia relativa WCAG (0 = negro, 1 = blanco). Ojo: no es la L de
    HSL -- un cian y un azul con la misma L de HSL tienen luminancias muy
    distintas."""
    def lin(c):
        c = c / 255.0
        return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4
    return 0.2126*lin(rgb[0]) + 0.7152*lin(rgb[1]) + 0.0722*lin(rgb[2])


def contraste(rgb1, rgb2):
    L1, L2 = luminancia(rgb1), luminancia(rgb2)
    return (max(L1, L2) + 0.05) / (min(L1, L2) + 0.05)


def mejor_texto_sobre(rgb):
    oscuro = (18, 12, 24)
    claro  = (245, 240, 230)
    return oscuro if contraste(rgb, oscuro) >= contraste(rgb, claro) else claro


def asegurar_contraste(fg, bg, minimo=4.5, pasos=100, direccion=None):
    """Cambia solo la luminosidad de fg (tono y saturacion intactos) hasta que
    su contraste con bg llegue a 'minimo'. Sin 'direccion' prueba aclarar y
    oscurecer y se queda con el cambio mas pequeno que lo logra; con
    direccion=+1 solo aclara y con -1 solo oscurece. Si no se logra: sin
    direccion devuelve el de mayor contraste posible; con direccion devuelve
    el extremo (blanco/negro de ese tono), para que al mover despues el otro
    color hacia el extremo opuesto el minimo siempre se alcance."""
    if contraste(fg, bg) >= minimo:
        return fg

    h, s, l = rgb_a_hsl(fg)
    mejor_ok, mejor_ok_paso = None, None
    mejor, mejor_c = fg, contraste(fg, bg)
    fija = direccion is not None
    if fija:
        mejor = a_565(hsl_a_rgb(h, s, 100.0 if direccion > 0 else 0.0))

    for d in ((direccion,) if fija else (1, -1)):
        for i in range(1, pasos + 1):
            l_cand = l + d * (100.0 / pasos) * i
            if l_cand < 0 or l_cand > 100:
                l_cand = max(0.0, min(100.0, l_cand))
            cand = a_565(hsl_a_rgb(h, s, l_cand))
            c = contraste(cand, bg)
            if c >= minimo:
                if mejor_ok_paso is None or i < mejor_ok_paso:
                    mejor_ok, mejor_ok_paso = cand, i
                break
            if not fija and c > mejor_c:
                mejor, mejor_c = cand, c
            if l_cand in (0.0, 100.0):
                break

    return mejor_ok if mejor_ok is not None else mejor


# ---------- catálogo de roles ----------

def _sup_delta(ctx, mult):
    d = ctx["superficie_delta"] * mult
    return -d if ctx["modo_claro"] else d


ROLES_CATALOGO = {
    "background":   {"label": "background",   "gen": lambda c: c["fondo"],           "core": True},
    "surface":      {"label": "surface",      "gen": lambda c: ajustar_l(c["fondo"], _sup_delta(c, 1)), "core": True},
    "surfaceAlt":   {"label": "surfaceAlt",   "gen": lambda c: ajustar_l(c["fondo"], _sup_delta(c, 2)), "core": True},
    "border":       {"label": "border",       "gen": lambda c: mix(ajustar_l(c["fondo"], _sup_delta(c, 2)), c["primario"], 0.45)},
    "primary":      {"label": "primary",      "gen": lambda c: c["primario"],        "core": True},
    "accent":       {"label": "accent",       "gen": lambda c: c["acento"],          "core": True},
    "accentAlt":    {"label": "accentAlt",    "gen": lambda c: c["acentoAlt"] or ajustar_hsl(c["acento"], dh=-35, ds=-25)},
    "text":         {"label": "text",         "gen": lambda c: c["texto"],           "core": True},
    "textMuted":    {"label": "textMuted",    "gen": lambda c: mix(c["texto"], c["fondo"], 0.45), "core": True},
    "textOnAccent": {"label": "textOnAccent", "gen": lambda c: mejor_texto_sobre(c["acento"]), "core": True},
    "textOnPrimary":{"label": "textOnPrimary","gen": lambda c: mejor_texto_sobre(c["primario"])},
    "success":      {"label": "success",      "gen": lambda c: hsl_a_rgb(140, 55, 45)},
    "warning":      {"label": "warning",      "gen": lambda c: hsl_a_rgb(38, 85, 55)},
    "danger":       {"label": "danger",       "gen": lambda c: hsl_a_rgb(0, 70, 50)},
    "info":         {"label": "info",         "gen": lambda c: hsl_a_rgb(205, 70, 55)},
    "shadow":       {"label": "shadow",       "gen": lambda c: mix(c["fondo"], (0, 0, 0), 0.6)},
    "overlay":      {"label": "overlay",      "gen": lambda c: mix(c["fondo"], c["texto"], 0.08)},
    "link":         {"label": "link",         "gen": lambda c: ajustar_hsl(c["primario"], ds=15, dl=10)},
}

ROLES_DEFAULT = ["background", "surface", "surfaceAlt", "border",
                 "primary", "accent", "accentAlt",
                 "text", "textMuted", "textOnAccent"]


# ---------- análisis de semillas ----------

def analizar_semillas(colores):
    info = [{"rgb": c, "h": rgb_a_hsl(c)[0], "s": rgb_a_hsl(c)[1], "l": rgb_a_hsl(c)[2]}
            for c in colores]

    por_l = sorted(info, key=lambda x: x["l"])
    fondo = por_l[0]["rgb"]
    texto = por_l[-1]["rgb"]

    medios = por_l[1:-1] if len(por_l) > 2 else por_l

    if medios:
        acento_cand = max(medios, key=lambda x: x["s"])
        acento = acento_cand["rgb"]
        otros = [m for m in medios if m is not acento_cand]
    else:
        acento = info[0]["rgb"]
        otros = []

    if otros:
        primario = max(otros, key=lambda x: x["s"])["rgb"]
        acento_alt = otros[0]["rgb"] if len(otros) > 1 else None
    else:
        h, s, l = rgb_a_hsl(acento)
        primario = hsl_a_rgb(h + 180, min(80, s), max(25, l * 0.8))
        acento_alt = None

    return {"fondo": fondo, "primario": primario, "acento": acento,
            "acentoAlt": acento_alt, "texto": texto}


# Parejas texto/fondo tal como las dibuja el firmware de Purple_Deck, con el
# contraste minimo exigido (WCAG: 4.5 texto normal, 3.0 texto secundario o
# etiquetas grandes).
PARES_CONTRASTE = [
    ("text",          "background", 4.5),
    ("text",          "surface",    4.5),   # tarjetas
    ("text",          "primary",    4.5),   # botones de la rejilla
    ("textMuted",     "background", 3.0),
    ("textMuted",     "surface",    3.0),
    ("textMuted",     "surfaceAlt", 3.0),
    ("textOnAccent",  "accent",     4.5),
    ("textOnPrimary", "primary",    4.5),
    ("accent",        "surface",    3.0),   # etiquetas de tarjetas
    ("accent",        "surfaceAlt", 3.0),   # texto del boton presionado
    ("link",          "background", 3.0),
]

# Roles que van "del lado del texto" y roles que van "del lado del fondo".
# En un tema oscuro el lado del texto es el claro; en uno claro, el oscuro.
# Cada grupo solo se mueve hacia su propio extremo, asi que nunca se cruzan
# ni oscilan: en el peor caso el texto termina blanco y el fondo negro
# (21:1), por lo que el minimo siempre se alcanza.
MARGEN = 0.05

LADO_TEXTO = {"text", "textMuted", "link", "accent"}
LADO_FONDO = {"background", "surface", "surfaceAlt", "primary"}


def garantizar_contraste(roles, claro_arriba, rondas=100):
    """Corrige 'roles' (dict rol -> rgb) en sitio hasta que todas las parejas
    de PARES_CONTRASTE presentes cumplan su minimo. claro_arriba: True si el
    texto del tema es mas claro que el fondo (tema oscuro)."""
    d_texto = 1 if claro_arriba else -1
    for rol in roles:
        roles[rol] = a_565(roles[rol])

    def dir_de(rol):
        if rol in LADO_TEXTO:
            return d_texto
        if rol in LADO_FONDO:
            return -d_texto
        return None  # textOnAccent / textOnPrimary: libre, solo dependen de un fondo

    for _ in range(rondas):
        cambio = False
        for fg, bg, minimo in PARES_CONTRASTE:
            if fg not in roles or bg not in roles:
                continue
            minimo += MARGEN  # colchon por redondeos del panel
            if contraste(roles[fg], roles[bg]) >= minimo:
                continue
            cambio = True
            roles[fg] = asegurar_contraste(roles[fg], roles[bg], minimo, direccion=dir_de(fg))
            if contraste(roles[fg], roles[bg]) < minimo:
                roles[bg] = asegurar_contraste(roles[bg], roles[fg], minimo, direccion=dir_de(bg))
        if not cambio:
            break
    return roles


def reporte_contraste(roles):
    """Lista (fg, bg, contraste, minimo, ok) de cada pareja presente."""
    filas = []
    for fg, bg, minimo in PARES_CONTRASTE:
        if fg in roles and bg in roles:
            c = contraste(roles[fg], roles[bg])
            filas.append((fg, bg, c, minimo, c >= minimo - 1e-6))
    return filas


def derivar_tema(semillas, roles_salida, nombre="Custom",
                 modo_claro=False, superficie_delta=7.0):
    base = analizar_semillas(semillas)

    fondo, texto = base["fondo"], base["texto"]
    if modo_claro:
        fondo, texto = texto, fondo
    texto = asegurar_contraste(texto, fondo, 4.5)
    if contraste(texto, fondo) < 4.5:
        # fondo cerca del gris medio: ningun texto llega, se mueve el fondo
        # (antes de derivar superficies, para que estas lo sigan)
        fondo = asegurar_contraste(fondo, texto, 4.5)

    ctx = {
        "fondo": fondo, "texto": texto,
        "primario": base["primario"], "acento": base["acento"],
        "acentoAlt": base["acentoAlt"],
        "modo_claro": modo_claro,
        "superficie_delta": superficie_delta,
        "roles": {},
    }

    resultado = {}
    orden = [r for r in roles_salida if ROLES_CATALOGO.get(r, {}).get("core")] + \
            [r for r in roles_salida if not ROLES_CATALOGO.get(r, {}).get("core")]

    for rol in orden:
        if rol in resultado:
            continue
        gen = ROLES_CATALOGO.get(rol, {}).get("gen")
        rgb = gen(ctx) if gen else (128, 128, 128)
        resultado[rol] = rgb
        ctx["roles"][rol] = rgb

    garantizar_contraste(resultado, luminancia(texto) >= luminancia(fondo))
    return {"nombre": nombre, "roles": resultado}


# ---------- exportadores ----------

# Orden fijo de exportacion: coincide con el orden de los campos del struct
# Tema en temas.h del firmware (fondo, superficie, superficieAlt, primario,
# acento, acentoAlt, borde, texto, textoTenue, textoSobreAcento). Si el orden
# de salida no coincide con el del struct, cada valor cae en el campo
# equivocado al copiar y pegar -- por ejemplo "texto" termina llevando el
# color que se calculo para "textOnAccent", y el contraste que se garantizo
# para ese rol ya no aplica al campo donde quedo. Cualquier rol fuera de esta
# lista (success, shadow, link...) se agrega al final, en el orden en que se
# generaron.
ORDEN_EXPORT = ["background", "surface", "surfaceAlt", "primary", "accent",
                "accentAlt", "border", "text", "textMuted", "textOnAccent",
                "textOnPrimary", "link", "success", "warning", "danger",
                "info", "shadow", "overlay"]


def _roles_ordenados(tema):
    orden = [r for r in ORDEN_EXPORT if r in tema["roles"]]
    orden += [r for r in tema["roles"] if r not in orden]
    return orden


def exp_cpp(tema):
    lineas = ["  {"]
    lineas.append(f'    /* nombre           */ "{tema["nombre"]}",')
    for rol in _roles_ordenados(tema):
        rgb = tema["roles"][rol]
        r, g, b = (int(round(c)) for c in rgb)
        v565 = rgb_a_rgb565(rgb)
        lineas.append(f"    /* {rol:<16} */ rgb565({r:3d}, {g:3d}, {b:3d}),   // {rgb_a_hex(rgb)}  ->  0x{v565:04X}")
    lineas.append("  },")
    return "\n".join(lineas)


def exp_json(tema):
    data = {
        "nombre": tema["nombre"],
        "colores": {rol: rgb_a_hex(tema["roles"][rol]) for rol in _roles_ordenados(tema)},
    }
    return json.dumps(data, indent=2, ensure_ascii=False)


def exp_css(tema):
    lineas = [":root {"]
    for rol in _roles_ordenados(tema):
        lineas.append(f"  --{rol}: {rgb_a_hex(tema['roles'][rol])};")
    lineas.append("}")
    return "\n".join(lineas)


def exp_hex(tema):
    lineas = [f"# Tema: {tema['nombre']}"]
    for rol in _roles_ordenados(tema):
        lineas.append(f"{rol}: {rgb_a_hex(tema['roles'][rol])}")
    return "\n".join(lineas)


EXPORTADORES = {
    "C++ (RGB565)": exp_cpp,
    "JSON":         exp_json,
    "CSS":          exp_css,
    "Hex list":     exp_hex,
}

EXTENSIONES = {
    "C++ (RGB565)": ".h",
    "JSON":         ".json",
    "CSS":          ".css",
    "Hex list":     ".txt",
}


# ---------- GUI ----------

PRESETS = {
    "Afton":     ["#6D28A8", "#C9A227", "#F3E9D8"],
    "Bosque":    ["#2E7D5B", "#E8B84B", "#E6F0E9"],
    "Cyberpunk": ["#7B2CBF", "#FF2E97", "#F0E6FF"],
    "Café":      ["#8B5E34", "#D4A574", "#F0E2CE"],
    "Océano":    ["#0A2540", "#2EC4B6", "#E0FBFC"],
    "Minimal":   ["#111111", "#FFFFFF"],
}


# ---------------------------------------------------------------------------
#  Pestana: generador de temas
# ---------------------------------------------------------------------------
def _css_color(rgb, defecto="#888888"):
    return rgb_a_hex(rgb) if rgb else defecto


class PanelTemas(QtWidgets.QWidget):
    def __init__(self, log):
        super().__init__()
        self.log_fn = log
        self.semillas = ["#6D28A8", "#C9A227", "#F3E9D8"]
        self.tema = None
        self._bloq = False
        self._ui()
        self._pintar_semillas()
        self._recalcular()

    # --- construccion ---
    def _ui(self):
        raiz = QtWidgets.QHBoxLayout(self)

        izq = QtWidgets.QVBoxLayout()
        izq_w = QtWidgets.QWidget()
        izq_w.setLayout(izq)
        izq_w.setFixedWidth(300)
        raiz.addWidget(izq_w)

        t = QtWidgets.QLabel("Generador de temas")
        t.setObjectName("titulo")
        izq.addWidget(t)
        h = QtWidgets.QLabel("2-5 colores de entrada, 3-10 roles de salida")
        h.setObjectName("ayuda")
        izq.addWidget(h)

        form = QtWidgets.QFormLayout()
        self.nombre = QtWidgets.QLineEdit("Custom")
        self.nombre.textChanged.connect(self._recalcular)
        self.preset = QtWidgets.QComboBox()
        self.preset.addItem("(elige)")
        self.preset.addItems(list(PRESETS.keys()))
        self.preset.currentTextChanged.connect(self._preset)
        form.addRow("Nombre", self.nombre)
        form.addRow("Preset", self.preset)
        izq.addLayout(form)

        izq.addWidget(QtWidgets.QLabel("Colores semilla (clic para cambiar):"))
        self.caja_semillas = QtWidgets.QVBoxLayout()
        izq.addLayout(self.caja_semillas)
        fila = QtWidgets.QHBoxLayout()
        b1 = QtWidgets.QPushButton("+ Anadir")
        b1.clicked.connect(self._add)
        b2 = QtWidgets.QPushButton("- Quitar")
        b2.clicked.connect(self._quitar)
        fila.addWidget(b1)
        fila.addWidget(b2)
        izq.addLayout(fila)

        izq.addWidget(QtWidgets.QLabel("Roles de salida (3-10):"))
        self.roles = QtWidgets.QListWidget()
        self.roles.setMaximumHeight(170)
        for rol in ROLES_CATALOGO:
            it = QtWidgets.QListWidgetItem(ROLES_CATALOGO[rol]["label"])
            it.setFlags(it.flags() | QtCore.Qt.ItemIsUserCheckable)
            it.setCheckState(QtCore.Qt.Checked if rol in ROLES_DEFAULT else QtCore.Qt.Unchecked)
            it.setData(QtCore.Qt.UserRole, rol)
            self.roles.addItem(it)
        self.roles.itemChanged.connect(self._rol_cambio)
        izq.addWidget(self.roles)

        self.claro = QtWidgets.QCheckBox("Modo claro")
        self.claro.toggled.connect(self._recalcular)
        izq.addWidget(self.claro)
        fd = QtWidgets.QHBoxLayout()
        fd.addWidget(QtWidgets.QLabel("Delta luminosidad superficies"))
        self.delta = QtWidgets.QDoubleSpinBox()
        self.delta.setRange(3, 20)
        self.delta.setDecimals(0)
        self.delta.setValue(7)
        self.delta.valueChanged.connect(self._recalcular)
        fd.addWidget(self.delta)
        izq.addLayout(fd)

        izq.addWidget(QtWidgets.QLabel("Formato de export"))
        self.formato = QtWidgets.QComboBox()
        self.formato.addItems(list(EXPORTADORES.keys()))
        self.formato.currentIndexChanged.connect(self._salida)
        izq.addWidget(self.formato)
        bc = QtWidgets.QPushButton("Copiar salida")
        bc.clicked.connect(self._copiar)
        bg = QtWidgets.QPushButton("Guardar...")
        bg.clicked.connect(self._guardar)
        izq.addWidget(bc)
        izq.addWidget(bg)
        izq.addStretch(1)

        der = QtWidgets.QVBoxLayout()
        raiz.addLayout(der, 1)
        self.mock = QtWidgets.QFrame()
        ml = QtWidgets.QVBoxLayout(self.mock)
        ml.setContentsMargins(0, 0, 0, 0)
        ml.setSpacing(0)
        self.m_hdr = QtWidgets.QFrame()
        hl = QtWidgets.QHBoxLayout(self.m_hdr)
        self.m_tit = QtWidgets.QLabel("Preview")
        self.m_badge = QtWidgets.QLabel(" NUEVO ")
        self.m_hora = QtWidgets.QLabel("12:34")
        hl.addWidget(self.m_tit)
        hl.addWidget(self.m_badge)
        hl.addStretch(1)
        hl.addWidget(self.m_hora)
        ml.addWidget(self.m_hdr)
        self.m_cuerpo = QtWidgets.QFrame()
        cl = QtWidgets.QVBoxLayout(self.m_cuerpo)
        self.m_card = QtWidgets.QFrame()
        kl = QtWidgets.QVBoxLayout(self.m_card)
        self.m_ct = QtWidgets.QLabel("Panel principal")
        self.m_cd = QtWidgets.QLabel("Texto secundario de ejemplo")
        kl.addWidget(self.m_ct)
        kl.addWidget(self.m_cd)
        fb = QtWidgets.QHBoxLayout()
        self.m_b1 = QtWidgets.QLabel("  Accion  ")
        self.m_b2 = QtWidgets.QLabel("  Destacar  ")
        self.m_b3 = QtWidgets.QLabel("  Alt  ")
        for b in (self.m_b1, self.m_b2, self.m_b3):
            fb.addWidget(b)
        fb.addStretch(1)
        kl.addLayout(fb)
        cl.addWidget(self.m_card)
        self.m_filas = []
        for txt in ("Elemento uno", "Elemento dos", "Elemento tres"):
            f = QtWidgets.QFrame()
            fl = QtWidgets.QHBoxLayout(f)
            pto, lb, fl_ = QtWidgets.QLabel("●"), QtWidgets.QLabel(txt), QtWidgets.QLabel("›")
            fl.addWidget(pto)
            fl.addWidget(lb, 1)
            fl.addWidget(fl_)
            cl.addWidget(f)
            self.m_filas.append((f, pto, lb, fl_))
        cl.addStretch(1)
        ml.addWidget(self.m_cuerpo, 1)
        der.addWidget(self.mock, 1)

        self.fila_sw = QtWidgets.QHBoxLayout()
        der.addLayout(self.fila_sw)
        self.contraste = QtWidgets.QLabel()
        self.contraste.setObjectName("ayuda")
        self.contraste.setWordWrap(True)
        der.addWidget(self.contraste)
        self.salida = QtWidgets.QPlainTextEdit()
        self.salida.setReadOnly(True)
        self.salida.setMaximumHeight(130)
        self.salida.setFont(QtGui.QFontDatabase.systemFont(QtGui.QFontDatabase.FixedFont))
        der.addWidget(self.salida)

    # --- semillas y roles ---
    def _pintar_semillas(self):
        while self.caja_semillas.count():
            w = self.caja_semillas.takeAt(0).widget()
            if w:
                w.setParent(None)
                w.deleteLater()
        for i, c in enumerate(self.semillas):
            b = QtWidgets.QPushButton(c)
            b.setStyleSheet(f"background:{c}; color:{mejor_hex_texto(c)}; border:1px solid #888;")
            b.clicked.connect(lambda _=False, i=i: self._elegir(i))
            self.caja_semillas.addWidget(b)

    def _elegir(self, i):
        c = QtWidgets.QColorDialog.getColor(QtGui.QColor(self.semillas[i]), self, "Elige un color")
        if c.isValid():
            self.semillas[i] = c.name().upper()
            self._pintar_semillas()
            self._recalcular()

    def _add(self):
        if len(self.semillas) >= 5:
            QtWidgets.QMessageBox.information(self, "Limite", "Maximo 5 colores.")
            return
        self.semillas.append("#808080")
        self._pintar_semillas()
        self._recalcular()

    def _quitar(self):
        if len(self.semillas) <= 2:
            QtWidgets.QMessageBox.information(self, "Limite", "Minimo 2 colores.")
            return
        self.semillas.pop()
        self._pintar_semillas()
        self._recalcular()

    def _preset(self, n):
        if n in PRESETS:
            self.semillas = list(PRESETS[n])
            self._bloq = True
            self.nombre.setText(n)
            self._bloq = False
            self._pintar_semillas()
            self._recalcular()

    def _roles_activos(self):
        return [self.roles.item(i).data(QtCore.Qt.UserRole) for i in range(self.roles.count())
                if self.roles.item(i).checkState() == QtCore.Qt.Checked]

    def _rol_cambio(self, it):
        if self._bloq:
            return
        n = len(self._roles_activos())
        if n < 3 or n > 10:
            self._bloq = True
            it.setCheckState(QtCore.Qt.Unchecked if it.checkState() == QtCore.Qt.Checked else QtCore.Qt.Checked)
            self._bloq = False
            QtWidgets.QMessageBox.information(self, "Roles", "Elige entre 3 y 10 roles.")
            return
        self._recalcular()

    # --- calculo ---
    def _recalcular(self, *_):
        if self._bloq or not hasattr(self, "salida"):
            return
        roles = self._roles_activos()
        if len(roles) < 3:
            roles = list(ROLES_DEFAULT)
        self.tema = derivar_tema([hex_a_rgb(c) for c in self.semillas], roles,
                                 self.nombre.text() or "Custom", self.claro.isChecked(), self.delta.value())
        self._preview()
        self._salida()

    def _preview(self):
        col = self.tema["roles"]
        H = _css_color
        bg = col.get("background", (30, 30, 30)); surf = col.get("surface", bg)
        surfA = col.get("surfaceAlt", surf); bord = col.get("border", surfA)
        prim = col.get("primary", bg); acc = col.get("accent", prim); accA = col.get("accentAlt", acc)
        txt = col.get("text", (240, 240, 240)); muted = col.get("textMuted", txt)
        onAcc = col.get("textOnAccent", (0, 0, 0))

        def est(w, fondo, fg=None, borde=None, extra=""):
            s = f"background:{H(fondo)};"
            if fg is not None:
                s += f"color:{H(fg)};"
            s += f"border:1px solid {H(borde)};" if borde is not None else "border:none;"
            w.setStyleSheet(s + extra)

        est(self.mock, bg)
        est(self.m_cuerpo, bg)
        est(self.m_hdr, surf)
        est(self.m_tit, surf, txt, extra="font-size:18px; font-weight:bold;")
        est(self.m_badge, acc, onAcc, extra="font-size:10px; font-weight:bold;")
        est(self.m_hora, surf, muted)
        est(self.m_card, surf, borde=bord)
        est(self.m_ct, surf, txt, extra="font-weight:bold;")
        est(self.m_cd, surf, muted)
        est(self.m_b1, prim, txt, extra="font-weight:bold; padding:5px;")
        est(self.m_b2, acc, onAcc, extra="font-weight:bold; padding:5px;")
        est(self.m_b3, accA, onAcc, extra="font-weight:bold; padding:5px;")
        for f, pto, lb, fl_ in self.m_filas:
            est(f, surfA, borde=bord)
            est(pto, surfA, prim)
            est(lb, surfA, txt)
            est(fl_, surfA, muted)

        filas = reporte_contraste(self.tema["roles"])
        malas = [f for f in filas if not f[4]]
        resumen = ("Contraste: todo OK (verificado en RGB565)" if not malas
                   else f"Contraste: {len(malas)} pareja(s) bajo el minimo")
        detalle = "   ".join(f"{'OK' if ok else 'MAL'} {fg}/{bgn} {c:.1f}" for fg, bgn, c, m, ok in filas)
        self.contraste.setText(resumen + "\n" + detalle)

        while self.fila_sw.count():
            w = self.fila_sw.takeAt(0).widget()
            if w:
                w.setParent(None)
                w.deleteLater()
        for rol, rgb in self.tema["roles"].items():
            b = QtWidgets.QPushButton(rol)
            b.setStyleSheet(f"background:{H(rgb)}; color:{mejor_hex_texto(H(rgb))}; border:1px solid #666; "
                            "padding:10px 0px; font-size:9px;")
            b.setToolTip("Clic: copiar " + H(rgb))
            b.clicked.connect(lambda _=False, c=H(rgb): QtWidgets.QApplication.clipboard().setText(c))
            self.fila_sw.addWidget(b)

    def _salida(self, *_):
        if self.tema is None:
            return
        self.salida.setPlainText(EXPORTADORES.get(self.formato.currentText(), exp_hex)(self.tema))

    def _copiar(self):
        QtWidgets.QApplication.clipboard().setText(self.salida.toPlainText())
        self.log_fn(f"Tema: salida ({self.formato.currentText()}) copiada")

    def _guardar(self):
        ext = EXTENSIONES.get(self.formato.currentText(), ".txt")
        ruta, _ = QtWidgets.QFileDialog.getSaveFileName(self, "Guardar tema",
                                                       f"tema_{self.tema['nombre'].lower()}{ext}")
        if ruta:
            with open(ruta, "w", encoding="utf-8") as f:
                f.write(self.salida.toPlainText())
            self.log_fn(f"Tema guardado: {ruta}")


def mejor_hex_texto(hexa):
    r, g, b = hex_a_rgb(hexa)
    return "#000000" if (0.299 * r + 0.587 * g + 0.114 * b) > 150 else "#FFFFFF"


# ---------------------------------------------------------------------------
#  Pestana: fotos para el visor del deck
# ---------------------------------------------------------------------------
EXT_FOTOS = (".jpg", ".jpeg", ".png", ".webp", ".bmp")


def convertir_fotos(carpeta, log):
    """JPG/PNG baseline en la subcarpeta 'convertidas' (el deck no lee JPG progresivo)."""
    try:
        from PIL import Image
    except ImportError:
        log("Falta Pillow. Corre:  pip install pillow")
        return
    destino = os.path.join(carpeta, "convertidas")
    os.makedirs(destino, exist_ok=True)
    archivos = [f for f in os.listdir(carpeta) if f.lower().endswith(EXT_FOTOS)]
    if not archivos:
        log("No encontre imagenes en: " + carpeta)
        return
    ok = 0
    for nombre in archivos:
        base, ext = os.path.splitext(nombre)
        es_png = ext.lower() == ".png"
        salida = os.path.join(destino, base + (".png" if es_png else ".jpg"))
        try:
            img = Image.open(os.path.join(carpeta, nombre))
            img.load()
        except Exception as e:
            log(f"  [ERROR] no se pudo abrir {nombre}: {e}")
            continue
        try:
            if es_png:
                img.save(salida, "PNG", optimize=True)
            else:
                if img.mode != "RGB":
                    img = img.convert("RGB")
                img.save(salida, "JPEG", quality=90, progressive=False, optimize=True)
            ok += 1
            log(f"  ok: {nombre} -> {os.path.basename(salida)}")
        except Exception as e:
            log(f"  [ERROR] no se pudo guardar {nombre}: {e}")
    log(f"Listo: {ok}/{len(archivos)} convertidas en {destino}")


class PanelFotos(QtWidgets.QWidget):
    def __init__(self, log):
        super().__init__()
        self.log_fn = log
        self.cfg = QtCore.QSettings("TeslaLab", "PurpleDeckTools")
        raiz = QtWidgets.QVBoxLayout(self)
        t = QtWidgets.QLabel("Fotos para el deck")
        t.setObjectName("titulo")
        raiz.addWidget(t)
        h = QtWidgets.QLabel("Convierte las imagenes de una carpeta a JPG baseline / PNG y las deja en la "
                             "subcarpeta 'convertidas', listas para copiar a la SD o al FTP del deck.")
        h.setObjectName("ayuda")
        h.setWordWrap(True)
        raiz.addWidget(h)
        fila = QtWidgets.QHBoxLayout()
        self.ruta = QtWidgets.QLineEdit(self.cfg.value("fotos_carpeta", ""))
        b = QtWidgets.QPushButton("Elegir carpeta...")
        b.clicked.connect(self._elegir)
        fila.addWidget(self.ruta, 1)
        fila.addWidget(b)
        raiz.addLayout(fila)
        go = QtWidgets.QPushButton("Convertir")
        go.clicked.connect(self._convertir)
        raiz.addWidget(go, 0, QtCore.Qt.AlignLeft)
        raiz.addStretch(1)

    def _elegir(self):
        d = QtWidgets.QFileDialog.getExistingDirectory(self, "Carpeta con las fotos", self.ruta.text())
        if d:
            self.ruta.setText(d)

    def _convertir(self):
        d = self.ruta.text().strip()
        if not os.path.isdir(d):
            QtWidgets.QMessageBox.warning(self, "Fotos", "Esa carpeta no existe.")
            return
        self.cfg.setValue("fotos_carpeta", d)
        QtWidgets.QApplication.setOverrideCursor(QtCore.Qt.WaitCursor)
        try:
            convertir_fotos(d, self.log_fn)
        finally:
            QtWidgets.QApplication.restoreOverrideCursor()


# ---------------------------------------------------------------------------
#  Ventana principal
# ---------------------------------------------------------------------------
class Ventana(QtWidgets.QMainWindow):
    def __init__(self, deck=None):
        super().__init__()
        self.deck = deck or Deck()
        self.cfg = QtCore.QSettings("TeslaLab", "PurpleDeckTools")
        self.setWindowTitle("Purple Deck - tools")
        c = QtWidgets.QWidget()
        self.setCentralWidget(c)
        raiz = QtWidgets.QVBoxLayout(c)

        barra = QtWidgets.QHBoxLayout()
        self.puertos = QtWidgets.QComboBox()
        self.puertos.setMinimumWidth(260)
        b_ref = QtWidgets.QPushButton("Actualizar")
        b_ref.clicked.connect(self._refrescar_puertos)
        self.b_con = QtWidgets.QPushButton("Conectar")
        self.b_con.clicked.connect(self.alternar_conexion)
        barra.addWidget(QtWidgets.QLabel("Puerto del deck:"))
        barra.addWidget(self.puertos)
        barra.addWidget(b_ref)
        barra.addWidget(self.b_con)
        barra.addStretch(1)
        self.estado = QtWidgets.QLabel("Sin conexion")
        self.estado.setObjectName("ayuda")
        barra.addWidget(self.estado)
        raiz.addLayout(barra)

        self.log = QtWidgets.QPlainTextEdit()
        self.log.setReadOnly(True)
        self.log.setMaximumHeight(100)
        self.p_slots = PanelMenus(self.deck, self.log.appendPlainText)
        self.p_slots.al_cambiar_sucio = lambda _s: self._titulo()
        self.p_red = PanelRed(self.deck, self.log.appendPlainText)
        self.p_temas = PanelTemas(self.log.appendPlainText)
        self.p_fotos = PanelFotos(self.log.appendPlainText)
        tabs = QtWidgets.QTabWidget()
        tabs.addTab(self.p_slots, "Menus")
        tabs.addTab(self.p_red, "Red")
        tabs.addTab(self.p_temas, "Temas")
        tabs.addTab(self.p_fotos, "Fotos")
        raiz.addWidget(tabs, 1)
        raiz.addWidget(self.log)
        self._refrescar_puertos()

    def _titulo(self):
        self.setWindowTitle("Purple Deck - tools" + ("  *" if self.p_slots.sucio else ""))

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
            self.b_con.setText("Conectar")
            self.puertos.setEnabled(True)
            self.p_slots.al_desconectar()
            self.p_red.al_desconectar()
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
            self.cfg.setValue("puerto", puerto)
            self.estado.setText(f"Conectado a {puerto}")
            self.b_con.setText("Desconectar")
            self.puertos.setEnabled(False)
            self.log.appendPlainText(f"Conectado a {puerto}")
            self.p_slots.al_conectar()
            self.p_red.al_conectar()
        except DeckError as e:
            self.deck.cerrar()
            self.estado.setText("Sin conexion")
            self.b_con.setText("Conectar")
            self.puertos.setEnabled(True)
            QtWidgets.QMessageBox.warning(self, "No se pudo conectar", str(e))
        finally:
            QtWidgets.QApplication.restoreOverrideCursor()

    def closeEvent(self, e):
        if not self.p_slots.puede_cerrar():
            e.ignore()
            return
        self.deck.cerrar()
        e.accept()


def main():
    app = QtWidgets.QApplication(sys.argv)
    app.setStyleSheet(ESTILO)
    w = Ventana()
    w.resize(1100, 740)
    w.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
