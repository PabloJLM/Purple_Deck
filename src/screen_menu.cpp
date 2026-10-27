#include "Pantallas/screen_menu.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"
#include "Drivers/menus.h"

// ---------------------------------------------------------------------------
//  Menu principal: botones de fabrica. Cual va en que pagina y en que orden lo
//  decide Drivers/menus.cpp (distribucion guardada, editable con tools.py).
//  Iconos de 64x64 en LittleFS: /menu1.png ... /menu11.png (solo este menu
//  lleva iconos). Si falta alguno, el boton sale solo con texto.
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

// Fijos + los 3 menus Custom (su nombre lo pone el usuario)
static Item s_items[N_FIJOS + 3];

const Item *screenMenuItems(uint8_t *n)
{
  memcpy(s_items, FIJOS, sizeof(FIJOS));
  for (uint8_t k = 0; k < 3; k++)
    s_items[N_FIJOS + k] = Item{ menuNombreCustom(k), "Macros propias", nullptr,
                                 ITEM_PANTALLA, (uint8_t)(SCREEN_CUSTOM1 + k), nullptr };
  *n = (uint8_t)(N_FIJOS + 3);
  return s_items;
}

void screenMenuEntrar() { menuEntrar(M_PRINCIPAL); }
void screenMenuLoop()   { menuLoop(M_PRINCIPAL);   }
void screenMenuSalir()  { menuSalir();             }
