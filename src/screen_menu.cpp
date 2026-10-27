#include "Pantallas/screen_menu.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"
#include "Drivers/slots.h"

// ---------------------------------------------------------------------------
//  Menu principal: 6 destinos en una rejilla de 3x2 que llena la pantalla.
//  Iconos de 64x64 en LittleFS: /menu1.png ... /menu8.png
//  Si falta alguno, el boton sale solo con texto.
// ---------------------------------------------------------------------------
static const Item FIJOS[] = {
  IT_PANT("Macros",         "Volumen y media",  "/menu1.png", SCREEN_MACROS),
  IT_PANT("SolidWorks",     "Vistas y rotar",   "/menu2.png", SCREEN_SOLIDWORKS),
  IT_PANT("LabVIEW",        "Run y diagrama",   "/menu3.png", SCREEN_LABVIEW),
  IT_PANT("Git /\nVS Code", "Commit y push",    "/menu4.png", SCREEN_GIT),
  IT_PANT("Ajustes",        "Brillo y reposo",  "/menu5.png", SCREEN_AJUSTES),
  IT_PANT("Blender",        "Vistas y render",  "/menu6.png", SCREEN_BLENDER),
  IT_PANT("Candle CNC",     "Correr y mover",   "/menu11.png", SCREEN_CANDLE),
  IT_PANT("Pomodoro",       "Foco y descanso",  "/menu7.png", SCREEN_POMODORO),
  IT_PANT("Reloj",          "Alarmas y crono",  "/menu8.png", SCREEN_RELOJ),
  IT_ACC("Bloquear",        "Pantalla de reloj", "/menu9.png",  ACC_BLOQUEAR_YA),
  IT_ACC("Fotos",           "Slideshow de la SD", "/menu10.png", ACC_FOTOS_YA),
};

#define N_FIJOS (sizeof(FIJOS) / sizeof(FIJOS[0]))

// Los fijos + los espacios editables que no estan vacios (Drivers/slots.h)
static Item        s_items[N_FIJOS + SLOTS_N];
static RejillaCfg  s_cfg = { s_items, 0, 3, true, SCREEN_MENU, true };
static uint32_t    s_version = 0;

static void armar()
{
  memcpy(s_items, FIJOS, sizeof(FIJOS));
  s_cfg.n = (uint8_t)(N_FIJOS + slotsAgregarItems(s_items + N_FIJOS));
  s_version = slotsVersion();
}

void screenMenuEntrar() { armar(); rejillaEntrar(s_cfg); }
void screenMenuLoop()
{
  // si se edito un espacio por la terminal mientras el menu esta abierto
  if (s_version != slotsVersion()) {
    rejillaSalir();
    armar();
    rejillaEntrar(s_cfg);
  }
  rejillaLoop();
}
void screenMenuSalir()  { rejillaSalir();  }
