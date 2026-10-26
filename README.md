# PurpleDeck

![placa](https://img.shields.io/badge/placa-WT32--SC01%20Plus-6D28A8)
![framework](https://img.shields.io/badge/framework-Arduino-00979D)
![estado](https://img.shields.io/badge/estado-en%20desarrollo-C9A227)

Stream Deck basada en ESP32, incluye nueve botones configurables
con íconos propios y una interfaz en morado y dorado, con soporte para
múltiples temas de color 

El proyecto utiliza la placa WT32-SC01 Plus, que integra procesador,
pantalla y panel táctil en una sola pieza, por lo que no requiere soldadura
ni cableado adicional.

---

## Características

**Botonera.** Cuadrícula de 3x3 que ocupa toda la pantalla. Cada botón
muestra una imagen PNG propia y responde visualmente al ser presionado. La
acción asociada a cada botón se define en el código.

**Pantalla de reposo.** Tras un período de inactividad, la pantalla reduce su
brillo y muestra un reloj con la fecha y las notificaciones recientes. El
reloj puede presentarse en formato analógico o digital, y el formato se
alterna tocando el propio reloj. Un toque en cualquier otra zona restaura la
botonera.

**Hora automática.** La hora se sincroniza por internet al conectarse a una
red WiFi, por lo que no requiere ajuste manual.

---

## Requisitos

- Placa **WT32-SC01 Plus**
- Cable USB-C
- **Arduino IDE** con el soporte para ESP32 y la librería LovyanGFX
- Red WiFi

---

## Instalación

**1. Obtener el proyecto.** Descargar el repositorio y abrir `Purple_Deck.ino`
con el Arduino IDE.

**2. Configurar la red WiFi.** Copiar el archivo `secrets.example.h` como
`secrets.h` e indicar el nombre y la contraseña de la red:

```cpp
#define WIFI_SSID  "SSID"
#define WIFI_PASS  "CONTRASEÑA"
```

El archivo `secrets.h` está excluido del repositorio 

**3. Preparar las imágenes.** Colocar nueve archivos PNG en la carpeta
`data/`, nombrados `boton1.png` hasta `boton9.png`. Si falta alguno, el botón
correspondiente se muestra únicamente con texto.

El script incluido `convertir_iconos.py` ajusta las imágenes al tamaño
exacto del botón mediante una interfaz gráfica: permite cargar varias
imágenes, elegir el tamaño y exportarlas a la carpeta `data/`. Requiere
Python 3 y la librería Pillow (`pip install pillow`).

**4. Configurar la placa** en el menú *Herramientas* del Arduino IDE:

| Opción | Valor |
|---|---|
| Board | ESP32S3 Dev Module |
| PSRAM | Habilitada |
| Partition Scheme | Cualquiera que incluya SPIFFS o FATFS |

El esquema de particiones reserva el espacio de la placa donde se almacenan
las imágenes. Con un esquema que no lo incluya, los íconos no se cargan.

**5. Cargar las imágenes a la placa** con el plugin de LittleFS del Arduino
IDE (*Ctrl+Shift+P → "Upload LittleFS"* en la versión 2.x, o
*Herramientas → ESP32 Sketch Data Upload* en la 1.8.x). El monitor serie
debe permanecer cerrado durante la carga, ya que mantiene ocupado el puerto.

**6. Cargar el programa** con el botón de subida habitual.

En caso de fallo, el monitor serie a 115200 baudios informa si las imágenes
fueron encontradas, si hubo conexión WiFi y qué hora se recibió.

---

## Personalización

**Colores.** El tema por defecto se llama *Afton*, en morado y dorado,
inspirado en Purple Guy y Springtrap. Todos los temas se definen en
`src/temas.cpp`, como entradas del arreglo `TEMAS[]`.

Cada tema es un `struct Tema` (`src/Estaticos/temas.h`) con **10 campos en
este orden exacto**:

```
fondo, superficie, superficieAlt, primario, acento, acentoAlt,
borde, texto, textoTenue, textoSobreAcento
```

El orden importa: son solo valores posicionales, así que si un campo queda
en el lugar equivocado el programa sigue compilando pero usa, por ejemplo,
el color pensado para "texto sobre acento" como fondo de tarjeta — y el
contraste que se calculó para un rol ya no aplica al rol donde terminó. Así
se rompieron por un tiempo los temas "Light" y "Cyberpunk" (letras
invisibles en algunos botones).

**Generador de temas (`generador_temas.py`).** Script de escritorio
(Tkinter) para crear temas nuevos sin calcular colores a mano:

- Parte de 2 a 5 colores "semilla" (o un preset) y deriva automáticamente
  fondo, superficie(s), primario, acento(s) y texto.
- Vista previa en vivo con una maqueta de pantalla (cabecera, tarjeta,
  botones, lista) para ver el tema antes de usarlo.
- Fuerza un contraste mínimo (WCAG: 4.5:1 para texto normal, 3:1 para texto
  atenuado) entre cada color de texto y el fondo sobre el que se dibuja,
  ajustando solo su luminosidad — nunca su tono ni saturación. Con colores
  semilla razonables esto garantiza letras legibles; con una semilla muy
  cercana al gris medio puede no alcanzar el mínimo exacto, porque ningún
  blanco o negro llega a 4.5:1 contra un gris a medio camino.
- Exporta a C++ (bloque listo para pegar en `TEMAS[]`, ya en el orden
  correcto del struct), JSON, CSS o una lista de hex.
- Requiere Python 3 (usa solo Tkinter, incluido con Python en Windows).

Para agregar un tema: correr `python generador_temas.py`, elegir colores,
copiar el bloque C++ generado dentro de `TEMAS[]` en `temas.cpp`, y
listo — `NUM_TEMAS` se calcula solo.

**Acción de cada botón.** Se define al final de `screen_deck.cpp`, en el
lugar señalado con un comentario.

**Tiempo de inactividad.** Se ajusta con `IDLE_TIMEOUT_MS` en `config.h`.

**Reloj inicial.** Se selecciona con `RELOJ_POR_DEFECTO` en `config.h`.

---

<details>
<summary>Detalles técnicos</summary>

### Hardware

Módulo WT32-S3-WROVER-N16R2: ESP32-S3 con 16 MB de flash y 2 MB de PSRAM.
Pantalla ST7796 de 320x480 con bus paralelo de 8 bits y panel táctil
capacitivo FT5x06 por I2C. La placa incorpora además ranura microSD, salida
de audio y RS485, que este proyecto aún no utiliza.

| Señal | GPIO |
|---|---|
| LCD DB0–DB7 | 9, 46, 3, 8, 18, 17, 16, 15 |
| LCD_WR / LCD_RS / LCD_TE | 47 / 0 / 48 |
| LCD_RESET | 4 (compartido con el reset del táctil) |
| Backlight PWM | 45 |
| Táctil SDA / SCL / INT | 6 / 5 / 7 |
| microSD CS / MOSI / CLK / MISO | 41 / 40 / 39 / 38 |
| IO libres | 10, 11, 12, 13, 14, 21 |

### Notificaciones

Buffer circular de 8 elementos, donde el índice 0 corresponde a la más
reciente:

```cpp
notifAgregar("Titulo", "Texto", "10:05", NOTIF_MENSAJE);
```

Tipos disponibles: `NOTIF_INFO`, `NOTIF_MENSAJE`, `NOTIF_ALERTA`.

### Dibujo

Cada botón y la tarjeta del reloj se componen en un sprite antes de enviarse
a la pantalla, lo que evita el parpadeo. La pantalla de reposo redibuja
únicamente lo que cambia: el reloj cada segundo, la cabecera cada minuto y la
lista cuando cambia `notifVersion()`.

La hora se obtiene por NTP con zona horaria `CST6` (Guatemala, sin horario
de verano).

</details>

---

## Pendientes

- Funciones de Stream Deck basicas (multimedia y control)
- Funciones para VS Code
- Funciones para SolidWorks
- Funciones para LabVIEW
- Funciones para Scripting 
- Recepción de notificaciones desde la computadora
- Dos temas de color adicionales
- Uso del buzzer y de la microSD
- Pasar a Linux