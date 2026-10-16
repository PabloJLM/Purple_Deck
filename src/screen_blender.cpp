#include "Pantallas/screen_blender.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

static const Item ITEMS[] = {
  IT_ACC("Frontal",         nullptr, "/blender_1.png", ACC_BL_FRONTAL),
  IT_ACC("Lateral",         nullptr, "/blender_2.png", ACC_BL_LATERAL),
  IT_ACC("Superior",        nullptr, "/blender_3.png", ACC_BL_SUPERIOR),
  IT_ACC("Encuadrar",       nullptr, "/blender_4.png", ACC_BL_ENCUADRAR),
  IT_ACC("Modo\nedicion",   nullptr, "/blender_5.png", ACC_BL_EDICION),
  IT_ACC("Extruir",         nullptr, "/blender_6.png", ACC_BL_EXTRUIR),
  IT_ACC("Agregar",         nullptr, "/blender_7.png", ACC_BL_AGREGAR),
  IT_ACC("Render",          nullptr, "/blender_8.png", ACC_BL_RENDER),
  IT_ACC("Guardar",         nullptr, "/blender_9.png", ACC_BL_GUARDAR),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenBlenderEntrar() { rejillaEntrar(CFG); }
void screenBlenderLoop()   { rejillaLoop();      }
void screenBlenderSalir()  { rejillaSalir();     }
