#include "Pantallas/screen_menu.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// ---------------------------------------------------------------------------
//  Menu principal: 6 destinos en una rejilla de 3x2 que llena la pantalla.
//  Iconos de 64x64 en LittleFS: /menu1.png ... /menu8.png
//  Si falta alguno, el boton sale solo con texto.
// ---------------------------------------------------------------------------
static const Item ITEMS[] = {
  IT_PANT("Macros",         "Volumen y media",  "/menu1.png", SCREEN_MACROS),
  IT_PANT("SolidWorks",     "Vistas y rotar",   "/menu2.png", SCREEN_SOLIDWORKS),
  IT_PANT("LabVIEW",        "Run y diagrama",   "/menu3.png", SCREEN_LABVIEW),
  IT_PANT("Git /\nVS Code", "Commit y push",    "/menu4.png", SCREEN_GIT),
  IT_PANT("Ajustes",        "Brillo y reposo",  "/menu5.png", SCREEN_AJUSTES),
  IT_PANT("Blender",        "Vistas y render",  "/menu6.png", SCREEN_BLENDER),
  IT_PANT("Pomodoro",       "Foco y descanso",  "/menu7.png", SCREEN_POMODORO),
  IT_PANT("Reloj",          "Alarmas y crono",  "/menu8.png", SCREEN_RELOJ),
  IT_ACC("Bloquear",        "Pantalla de reloj", "/menu9.png",  ACC_BLOQUEAR_YA),
  IT_ACC("Fotos",           "Slideshow de la SD", "/menu10.png", ACC_FOTOS_YA),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,              // columnas
  true,           // paginada: 7 por pagina, Ant y Sig abajo
  SCREEN_MENU,    // (sin uso: no hay boton Atras)
  true            // abajo-izquierda = Ant
};

void screenMenuEntrar() { rejillaEntrar(CFG); }
void screenMenuLoop()   { rejillaLoop();      }
void screenMenuSalir()  { rejillaSalir();     }
