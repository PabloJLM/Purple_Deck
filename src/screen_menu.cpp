#include "Pantallas/screen_menu.h"
#include "Estaticos/rejilla.h"

// ---------------------------------------------------------------------------
//  Menu principal: 6 destinos en una rejilla de 3x2 que llena la pantalla.
//  Iconos de 64x64 en LittleFS: /menu1.png ... /menu6.png
//  Si falta alguno, el boton sale solo con texto.
// ---------------------------------------------------------------------------
static const Item ITEMS[] = {
  IT_PANT("Macros",         "Volumen y media",  "/menu1.png", SCREEN_MACROS),
  IT_PANT("SolidWorks",     "Vistas y rebuild", "/menu2.png", SCREEN_SOLIDWORKS),
  IT_PANT("LabVIEW",        "Run y diagrama",   "/menu3.png", SCREEN_LABVIEW),
  IT_PANT("Git /\nVS Code", "Commit y push",    "/menu4.png", SCREEN_GIT),
  IT_PANT("Ajustes",        "Brillo y reposo",  "/menu5.png", SCREEN_AJUSTES),
  IT_PANT("Arduino\nIDE",   "Compilar y subir", "/menu6.png", SCREEN_ARDUINO),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,              // columnas
  false,          // no paginada: los 6 llenan la pantalla
  SCREEN_MENU     // (sin uso: no hay boton Atras)
};

void screenMenuEntrar() { rejillaEntrar(CFG); }
void screenMenuLoop()   { rejillaLoop();      }
void screenMenuSalir()  { rejillaSalir();     }
