import os
import sys
import traceback

try:
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox, colorchooser
except Exception:
    print("No se encontro tkinter. En Windows reinstala Python marcando 'tcl/tk'.")
    raise

try:
    from PIL import Image, ImageOps, ImageTk
except ImportError:
    import tkinter as tk
    from tkinter import messagebox
    _r = tk.Tk(); _r.withdraw()
    messagebox.showerror("Falta Pillow",
                         "Este programa necesita Pillow.\n\n"
                         "Abri una terminal y corre:\n\n    pip install pillow")
    sys.exit(1)


PRESETS = {
    "Icono + texto (52 x 52)":       (52, 52),      
    "Icono ancho (138 x 54)":        (138, 54),
    "Icono grande (64 x 64)":        (64, 64),
    "Boton completo (154 x 100)":    (154, 100),    
    "Personalizado...":              None,
}

EXTS = [("Imagenes", "*.png *.jpg *.jpeg *.bmp *.webp *.gif"), ("Todos", "*.*")]

COLOR_BTN_DEFAULT = "#6B21A8"  


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Convertidor de iconos - Botonera WT32-SC01-Plus")
        self.geometry("880x560")
        self.minsize(820, 520)

        self.rutas = []                
        self._preview_img = None       
        self.color_fondo = COLOR_BTN_DEFAULT

        base = os.path.dirname(os.path.abspath(__file__))
        self.var_salida = tk.StringVar(value=os.path.join(base, "data"))
        self.var_preset = tk.StringVar(value="Icono + texto (52 x 52)")
        self.var_w = tk.StringVar(value="52")
        self.var_h = tk.StringVar(value="52")
        self.var_modo = tk.StringVar(value="contener")
        self.var_fondo = tk.StringVar(value="color") 
        self.var_nombres = tk.StringVar(value="boton") 

        self._build_ui()
        self._aplicar_preset()

    def _build_ui(self):
        pad = dict(padx=8, pady=4)

        cont = ttk.Frame(self, padding=10)
        cont.pack(fill="both", expand=True)
        cont.columnconfigure(0, weight=3)
        cont.columnconfigure(1, weight=2)
        cont.rowconfigure(0, weight=1)

        izq = ttk.LabelFrame(cont, text="1. Imagenes (el orden = boton 1, 2, 3...)", padding=8)
        izq.grid(row=0, column=0, sticky="nsew", padx=(0, 8))
        izq.rowconfigure(0, weight=1)
        izq.columnconfigure(0, weight=1)

        self.lista = tk.Listbox(izq, activestyle="dotbox")
        self.lista.grid(row=0, column=0, sticky="nsew")
        self.lista.bind("<<ListboxSelect>>", lambda e: self._preview())

        sb = ttk.Scrollbar(izq, orient="vertical", command=self.lista.yview)
        sb.grid(row=0, column=1, sticky="ns")
        self.lista.config(yscrollcommand=sb.set)

        barra = ttk.Frame(izq)
        barra.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(8, 0))
        ttk.Button(barra, text="Agregar...", command=self.agregar).pack(side="left")
        ttk.Button(barra, text="Quitar", command=self.quitar).pack(side="left", padx=4)
        ttk.Button(barra, text="Limpiar", command=self.limpiar).pack(side="left")
        ttk.Button(barra, text="Subir", command=lambda: self.mover(-1)).pack(side="right")
        ttk.Button(barra, text="Bajar", command=lambda: self.mover(1)).pack(side="right", padx=4)


        der = ttk.Frame(cont)
        der.grid(row=0, column=1, sticky="nsew")
        der.columnconfigure(0, weight=1)

        op = ttk.LabelFrame(der, text="2. Tamano y ajuste", padding=8)
        op.grid(row=0, column=0, sticky="ew")
        op.columnconfigure(1, weight=1)

        ttk.Label(op, text="Preset:").grid(row=0, column=0, sticky="w", **pad)
        cb = ttk.Combobox(op, textvariable=self.var_preset, values=list(PRESETS.keys()),
                          state="readonly")
        cb.grid(row=0, column=1, columnspan=3, sticky="ew", **pad)
        cb.bind("<<ComboboxSelected>>", lambda e: self._aplicar_preset())

        ttk.Label(op, text="Ancho:").grid(row=1, column=0, sticky="w", **pad)
        self.e_w = ttk.Entry(op, textvariable=self.var_w, width=6)
        self.e_w.grid(row=1, column=1, sticky="w", **pad)
        ttk.Label(op, text="Alto:").grid(row=1, column=2, sticky="e", **pad)
        self.e_h = ttk.Entry(op, textvariable=self.var_h, width=6)
        self.e_h.grid(row=1, column=3, sticky="w", **pad)

        ttk.Label(op, text="Ajuste:").grid(row=2, column=0, sticky="w", **pad)
        f = ttk.Frame(op); f.grid(row=2, column=1, columnspan=3, sticky="w")
        ttk.Radiobutton(f, text="Contener (no recorta)", value="contener",
                        variable=self.var_modo, command=self._preview).pack(side="left")
        ttk.Radiobutton(f, text="Recortar (llena)", value="recortar",
                        variable=self.var_modo, command=self._preview).pack(side="left", padx=8)

        ttk.Label(op, text="Fondo:").grid(row=3, column=0, sticky="w", **pad)
        f2 = ttk.Frame(op); f2.grid(row=3, column=1, columnspan=3, sticky="w")
        ttk.Radiobutton(f2, text="Color", value="color",
                        variable=self.var_fondo, command=self._preview).pack(side="left")
        self.btn_color = tk.Button(f2, text="   ", bg=self.color_fondo, width=3,
                                   relief="ridge", command=self.elegir_color)
        self.btn_color.pack(side="left", padx=6)
        ttk.Radiobutton(f2, text="Transparente", value="transparente",
                        variable=self.var_fondo, command=self._preview).pack(side="left", padx=8)

        sal = ttk.LabelFrame(der, text="3. Salida", padding=8)
        sal.grid(row=1, column=0, sticky="ew", pady=(8, 0))
        sal.columnconfigure(0, weight=1)

        ttk.Entry(sal, textvariable=self.var_salida).grid(row=0, column=0, sticky="ew", **pad)
        ttk.Button(sal, text="...", width=3, command=self.elegir_salida).grid(row=0, column=1, **pad)

        f3 = ttk.Frame(sal); f3.grid(row=1, column=0, columnspan=2, sticky="w")
        ttk.Radiobutton(f3, text="Renombrar boton1.png, boton2.png...", value="boton",
                        variable=self.var_nombres).pack(side="left")
        ttk.Radiobutton(f3, text="Conservar nombre", value="original",
                        variable=self.var_nombres).pack(side="left", padx=8)

        pv = ttk.LabelFrame(der, text="Vista previa (2x)", padding=8)
        pv.grid(row=2, column=0, sticky="nsew", pady=(8, 0))
        der.rowconfigure(2, weight=1)
        self.canvas = tk.Canvas(pv, bg="#12071C", highlightthickness=0, height=180)
        self.canvas.pack(fill="both", expand=True)


        low = ttk.Frame(self, padding=(10, 0, 10, 10))
        low.pack(fill="both")
        low.columnconfigure(0, weight=1)

        self.log = tk.Text(low, height=6, wrap="none", state="disabled",
                           bg="#1b1b1b", fg="#dcdcdc", insertbackground="#dcdcdc")
        self.log.grid(row=0, column=0, sticky="ew")

        ttk.Button(low, text="CONVERTIR", command=self.convertir).grid(
            row=0, column=1, sticky="ns", padx=(8, 0))

        self._log("Listo. Agrega imagenes y presiona CONVERTIR.")
        self._log("Consejo: la carpeta de salida debe llamarse 'data' y estar")
        self._log("junto al .ino para que el plugin LittleFS la suba a la placa.")


    def _log(self, txt):
        self.log.config(state="normal")
        self.log.insert("end", txt + "\n")
        self.log.see("end")
        self.log.config(state="disabled")

    def _aplicar_preset(self):
        val = PRESETS.get(self.var_preset.get())
        estado = "normal" if val is None else "readonly"
        if val:
            self.var_w.set(str(val[0]))
            self.var_h.set(str(val[1]))
        self.e_w.config(state=estado)
        self.e_h.config(state=estado)
        self._preview()

    def _tamano(self):
        try:
            w = max(1, min(480, int(self.var_w.get())))
            h = max(1, min(320, int(self.var_h.get())))
            return w, h
        except ValueError:
            return 96, 96

    def elegir_color(self):
        c = colorchooser.askcolor(color=self.color_fondo, title="Color de fondo")
        if c and c[1]:
            self.color_fondo = c[1]
            self.btn_color.config(bg=self.color_fondo)
            self.var_fondo.set("color")
            self._preview()

    def elegir_salida(self):
        d = filedialog.askdirectory(title="Carpeta de salida (recomendado: .../sketch/data)")
        if d:
            self.var_salida.set(d)

    def agregar(self):
        archivos = filedialog.askopenfilenames(title="Elegi las imagenes", filetypes=EXTS)
        for a in archivos:
            if a not in self.rutas:
                self.rutas.append(a)
        self._refrescar()

    def quitar(self):
        sel = list(self.lista.curselection())
        for i in reversed(sel):
            del self.rutas[i]
        self._refrescar()

    def limpiar(self):
        self.rutas.clear()
        self._refrescar()

    def mover(self, delta):
        sel = self.lista.curselection()
        if not sel:
            return
        i = sel[0]
        j = i + delta
        if 0 <= j < len(self.rutas):
            self.rutas[i], self.rutas[j] = self.rutas[j], self.rutas[i]
            self._refrescar(seleccion=j)

    def _refrescar(self, seleccion=None):
        self.lista.delete(0, "end")
        for i, r in enumerate(self.rutas, start=1):
            self.lista.insert("end", f"{i:>2}. {os.path.basename(r)}")
        if self.rutas:
            idx = seleccion if seleccion is not None else 0
            self.lista.selection_clear(0, "end")
            self.lista.selection_set(idx)
            self.lista.activate(idx)
        self._preview()


    def _procesar(self, ruta):
        """Devuelve la imagen ya convertida al tamano/modo elegido."""
        w, h = self._tamano()
        img = Image.open(ruta)
        img = ImageOps.exif_transpose(img)
        img = img.convert("RGBA")

        if self.var_modo.get() == "recortar":
            img = ImageOps.fit(img, (w, h), method=Image.LANCZOS, centering=(0.5, 0.5))
            lienzo = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            lienzo.alpha_composite(img)
        elif hasattr(ImageOps, "contain"):
            img = ImageOps.contain(img, (w, h), method=Image.LANCZOS)
            lienzo = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            lienzo.alpha_composite(img, ((w - img.width) // 2, (h - img.height) // 2))
        else:   # Pillow viejo (< 8.3)
            esc = min(w / img.width, h / img.height)
            img = img.resize((max(1, int(img.width * esc)), max(1, int(img.height * esc))),
                             Image.LANCZOS)
            lienzo = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            lienzo.alpha_composite(img, ((w - img.width) // 2, (h - img.height) // 2))

        if self.var_fondo.get() == "color":
            r = int(self.color_fondo[1:3], 16)
            g = int(self.color_fondo[3:5], 16)
            b = int(self.color_fondo[5:7], 16)
            fondo = Image.new("RGBA", (w, h), (r, g, b, 255))
            fondo.alpha_composite(lienzo)
            return fondo.convert("RGB")

        return lienzo  

    def _preview(self):
        self.canvas.delete("all")
        sel = self.lista.curselection()
        if not sel or not self.rutas:
            self.canvas.create_text(10, 10, anchor="nw", fill="#8b6ba8",
                                    text="Selecciona una imagen de la lista")
            return
        try:
            img = self._procesar(self.rutas[sel[0]])
            vista = img.resize((img.width * 2, img.height * 2), Image.NEAREST)
            self._preview_img = ImageTk.PhotoImage(vista)
            cw = self.canvas.winfo_width() or 300
            ch = self.canvas.winfo_height() or 180
            self.canvas.create_image(cw // 2, ch // 2, image=self._preview_img)
            w, h = self._tamano()
            self.canvas.create_text(6, 6, anchor="nw", fill="#c084fc",
                                    text=f"{w} x {h} px")
        except Exception as e:
            self.canvas.create_text(10, 10, anchor="nw", fill="#ff8080",
                                    text=f"No se pudo abrir: {e}")

    def convertir(self):
        if not self.rutas:
            messagebox.showinfo("Sin imagenes", "Primero agrega al menos una imagen.")
            return

        destino = self.var_salida.get().strip()
        if not destino:
            messagebox.showinfo("Sin carpeta", "Elegi una carpeta de salida.")
            return
        os.makedirs(destino, exist_ok=True)

        w, h = self._tamano()
        total = 0
        ok = 0
        self._log("-" * 58)
        self._log(f"Convirtiendo {len(self.rutas)} imagen(es) a {w}x{h} -> {destino}")

        for i, ruta in enumerate(self.rutas, start=1):
            try:
                img = self._procesar(ruta)
                if self.var_nombres.get() == "boton":
                    nombre = f"boton{i}.png"
                else:
                    base = os.path.splitext(os.path.basename(ruta))[0]
                    nombre = f"{base[:24]}.png"       # LittleFS: nombres cortos
                salida = os.path.join(destino, nombre)
                img.save(salida, format="PNG", optimize=True)
                tam = os.path.getsize(salida)
                total += tam
                ok += 1
                self._log(f"  OK  {nombre:<20} {tam/1024:6.1f} KB   ({os.path.basename(ruta)})")
            except Exception as e:
                self._log(f"  ERROR en {os.path.basename(ruta)}: {e}")
                traceback.print_exc()

        self._log(f"Listo: {ok}/{len(self.rutas)} archivos, {total/1024:.1f} KB en total.")
        if total > 900 * 1024:
            self._log("  Ojo: mas de ~900 KB. Revisa el tamano de la particion LittleFS.")
        self._log("Ahora sube la carpeta 'data' con el plugin LittleFS (Herramientas ->")
        self._log("ESP32 Sketch Data Upload) y ya las podes dibujar con drawPngFile().")


if __name__ == "__main__":
    App().mainloop()
