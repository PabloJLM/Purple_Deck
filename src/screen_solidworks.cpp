#include "Pantallas/screen_solidworks.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

static const Item ITEMS[] = {
  IT_ACC("Ajustar\nvista", nullptr, "/solidworks_1.png", ACC_SW_AJUSTAR),
  IT_ACC("Frontal",        nullptr, "/solidworks_2.png", ACC_SW_FRONTAL),
  IT_ACC("Superior",       nullptr, "/solidworks_3.png", ACC_SW_SUPERIOR),
  IT_ACC("Lateral",        nullptr, "/solidworks_4.png", ACC_SW_LATERAL),
  IT_ACC("Extruir",        nullptr, "/solidworks_5.png", ACC_SW_EXTRUIR),
  IT_ACC("Sketch",         nullptr, "/solidworks_6.png", ACC_SW_SKETCH),
  IT_PANT("Rotar",   "Con el dedo", "/solidworks_7.png", SCREEN_SW_ROTAR),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenSolidworksEntrar() { rejillaEntrar(CFG); }
void screenSolidworksLoop()   { rejillaLoop();      }
void screenSolidworksSalir()  { rejillaSalir();     }
