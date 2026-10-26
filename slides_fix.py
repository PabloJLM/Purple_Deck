"""
reencode_slides.py - vuelve a guardar fotos como JPEG baseline (o PNG normal)
para que el decodificador del deck las pueda leer.

Uso:
    python reencode_slides.py "C:\\ruta\\a\\slides"

Deja los originales intactos y pone las copias convertidas en una
subcarpeta "convertidas" dentro de esa misma carpeta.
"""
import os
import sys

try:
    from PIL import Image
except ImportError:
    print("Falta Pillow. Corre:  pip install pillow")
    sys.exit(1)

EXTENSIONES = (".jpg", ".jpeg", ".png", ".webp", ".bmp")


def convertir(carpeta):
    destino = os.path.join(carpeta, "convertidas")
    os.makedirs(destino, exist_ok=True)

    archivos = [f for f in os.listdir(carpeta) if f.lower().endswith(EXTENSIONES)]
    if not archivos:
        print("No encontre imagenes en:", carpeta)
        return

    ok = 0
    for nombre in archivos:
        origen = os.path.join(carpeta, nombre)
        base, ext = os.path.splitext(nombre)
        es_png = ext.lower() == ".png"
        salida = os.path.join(destino, base + (".png" if es_png else ".jpg"))

        try:
            img = Image.open(origen)
            img.load()
        except Exception as e:
            print(f"  [ERROR] no se pudo abrir {nombre}: {e}")
            continue

        try:
            if es_png:
                img.save(salida, "PNG", optimize=True)
            else:
                if img.mode != "RGB":
                    img = img.convert("RGB")
                img.save(salida, "JPEG", quality=90, progressive=False, optimize=True)
            ok += 1
            print(f"  ok: {nombre} -> {os.path.basename(salida)}")
        except Exception as e:
            print(f"  [ERROR] no se pudo guardar {nombre}: {e}")

    print(f"\nListo: {ok}/{len(archivos)} convertidas en {destino}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        carpeta = input("Carpeta con las fotos: ").strip().strip('"')
    else:
        carpeta = sys.argv[1]

    if not os.path.isdir(carpeta):
        print("No existe esa carpeta:", carpeta)
        sys.exit(1)

    convertir(carpeta)