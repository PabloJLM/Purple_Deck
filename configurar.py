import os
import re
import sys

try:
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox
except Exception:
    print("No se encontro tkinter. En Windows reinstala Python marcando 'tcl/tk'.")
    raise

CAMPOS = [
    ("WIFI_SSID", "Red WiFi (SSID)", False),
    ("WIFI_PASS", "Contrasena", True),
]

PLANTILLA = """#ifndef PURPLE_DECK_SECRETS_H_
#define PURPLE_DECK_SECRETS_H_
//  Generado por configurar.py. No se sube al repo (esta en .gitignore).

{lineas}

#endif  // PURPLE_DECK_SECRETS_H_
"""

BG = "#1b1626"
CARD = "#2a2140"
ACENTO = "#6B21A8"
TEXTO = "#eeeaf7"
TENUE = "#9a90b5"


def escapar(valor):
    return valor.replace("\\", "\\\\").replace('"', '\\"')


def desescapar(texto):
    return re.sub(r'\\(.)', r'\1', texto)


def leer_secretos(ruta):
    datos = {clave: "" for clave, _, _ in CAMPOS}
    if not os.path.isfile(ruta):
        return datos
    with open(ruta, encoding="utf-8") as f:
        contenido = f.read()
    for clave in datos:
        m = re.search(r'#define\s+' + clave + r'\s+"((?:[^"\\]|\\.)*)"', contenido)
        if m:
            datos[clave] = desescapar(m.group(1))
    return datos


def escribir_secretos(ruta, datos):
    lineas = "\n".join('#define {:<10} "{}"'.format(clave, escapar(datos[clave])) for clave, _, _ in CAMPOS)
    with open(ruta, "w", encoding="utf-8", newline="\n") as f:
        f.write(PLANTILLA.format(lineas=lineas))


def validar(datos):
    ssid = datos["WIFI_SSID"]
    clave = datos["WIFI_PASS"]
    if not ssid:
        return "El SSID no puede estar vacio.", False
    if len(ssid.encode("utf-8")) > 32:
        return "El SSID no puede pasar de 32 bytes.", False
    if clave and len(clave) < 8:
        return "Una contrasena WPA2 debe tener al menos 8 caracteres.", False
    if len(clave) > 63:
        return "La contrasena no puede pasar de 63 caracteres.", False
    return "", True


def carpeta_proyecto():
    base = os.path.dirname(os.path.abspath(__file__))
    if os.path.isdir(os.path.join(base, "src")):
        return base
    elegida = filedialog.askdirectory(title="Carpeta del proyecto Purple_Deck (la que tiene src)")
    if elegida and os.path.isdir(os.path.join(elegida, "src")):
        return elegida
    return None


class App(tk.Tk):
    def __init__(self, proyecto):
        super().__init__()
        self.title("Credenciales - Purple Deck")
        self.configure(bg=BG)
        self.resizable(False, False)
        self.ruta = os.path.join(proyecto, "src", "secrets.h")

        tk.Label(self, text="Credenciales del deck", bg=BG, fg=TEXTO,
                 font=("Segoe UI", 16, "bold")).grid(row=0, column=0, columnspan=2, padx=24, pady=(20, 4), sticky="w")
        tk.Label(self, text=self.ruta, bg=BG, fg=TENUE,
                 font=("Segoe UI", 8)).grid(row=1, column=0, columnspan=2, padx=24, pady=(0, 14), sticky="w")

        datos = leer_secretos(self.ruta)
        self.vars = {}
        self.entradas = {}
        for i, (clave, etiqueta, oculto) in enumerate(CAMPOS):
            tk.Label(self, text=etiqueta, bg=BG, fg=TENUE,
                     font=("Segoe UI", 10)).grid(row=2 + i, column=0, padx=(24, 10), pady=6, sticky="e")
            v = tk.StringVar(value=datos[clave])
            e = tk.Entry(self, textvariable=v, width=34, bg=CARD, fg=TEXTO, insertbackground=TEXTO,
                         relief="flat", font=("Segoe UI", 11), show="*" if oculto else "")
            e.grid(row=2 + i, column=1, padx=(0, 24), pady=6, ipady=5)
            self.vars[clave] = v
            self.entradas[clave] = (e, oculto)

        self.mostrar = tk.BooleanVar(value=False)
        tk.Checkbutton(self, text="Mostrar contrasena", variable=self.mostrar, command=self.alternar,
                       bg=BG, fg=TENUE, activebackground=BG, activeforeground=TEXTO,
                       selectcolor=CARD).grid(row=2 + len(CAMPOS), column=1, padx=(0, 24), sticky="w")

        tk.Button(self, text="Guardar", command=self.guardar, bg=ACENTO, fg="white", relief="flat",
                  activebackground="#7c2fc0", activeforeground="white", font=("Segoe UI", 11, "bold"),
                  padx=24, pady=8).grid(row=3 + len(CAMPOS), column=0, columnspan=2, pady=(16, 6))

        self.estado = tk.Label(self, text="", bg=BG, fg=TENUE, font=("Segoe UI", 9), wraplength=420)
        self.estado.grid(row=4 + len(CAMPOS), column=0, columnspan=2, padx=24, pady=(0, 18))

    def alternar(self):
        for e, oculto in self.entradas.values():
            if oculto:
                e.config(show="" if self.mostrar.get() else "*")

    def guardar(self):
        datos = {clave: v.get() for clave, v in self.vars.items()}
        msg, ok = validar(datos)
        if not ok:
            messagebox.showwarning("Revisa los datos", msg)
            return
        try:
            escribir_secretos(self.ruta, datos)
        except OSError as e:
            messagebox.showerror("No se pudo guardar", str(e))
            return
        self.estado.config(fg="#7ee0a0", text="Guardado. Recompila y sube el firmware para que el deck use los datos nuevos.")


def main():
    raiz = tk.Tk()
    raiz.withdraw()
    proyecto = carpeta_proyecto()
    raiz.destroy()
    if not proyecto:
        sys.exit(1)
    App(proyecto).mainloop()


if __name__ == "__main__":
    main()
