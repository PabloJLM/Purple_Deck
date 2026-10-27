#include "Pantallas/screen_blender.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"
#include "Drivers/menus.h"

static const Item ITEMS[] = {
  IT_ACC("Frontal",         nullptr, nullptr, ACC_BL_FRONTAL),
  IT_ACC("Lateral",         nullptr, nullptr, ACC_BL_LATERAL),
  IT_ACC("Superior",        nullptr, nullptr, ACC_BL_SUPERIOR),
  IT_ACC("Encuadrar",       nullptr, nullptr, ACC_BL_ENCUADRAR),
  IT_ACC("Modo\nedicion",   nullptr, nullptr, ACC_BL_EDICION),
  IT_ACC("Extruir",         nullptr, nullptr, ACC_BL_EXTRUIR),
  IT_ACC("Agregar",         nullptr, nullptr, ACC_BL_AGREGAR),
  IT_ACC("Render",          nullptr, nullptr, ACC_BL_RENDER),
  IT_ACC("Guardar",         nullptr, nullptr, ACC_BL_GUARDAR),
};

const Item *screenBlenderItems(uint8_t *n) { *n = sizeof(ITEMS) / sizeof(ITEMS[0]); return ITEMS; }

void screenBlenderEntrar() { menuEntrar(M_BLENDER); }
void screenBlenderLoop()   { menuLoop(M_BLENDER);   }
void screenBlenderSalir()  { menuSalir();    }
