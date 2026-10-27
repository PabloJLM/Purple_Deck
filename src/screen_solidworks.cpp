#include "Pantallas/screen_solidworks.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

static const Item ITEMS[] = {
  IT_ACC("Ajustar\nvista", nullptr, nullptr, ACC_SW_AJUSTAR),
  IT_ACC("Frontal",        nullptr, nullptr, ACC_SW_FRONTAL),
  IT_ACC("Superior",       nullptr, nullptr, ACC_SW_SUPERIOR),
  IT_ACC("Lateral",        nullptr, nullptr, ACC_SW_LATERAL),
  IT_ACC("Abrir",          nullptr, nullptr, ACC_SW_ABRIR),
  IT_ACC("Nuevo\narchivo", nullptr, nullptr, ACC_SW_NUEVO),
  IT_PANT("Rotar",         nullptr, nullptr, SCREEN_SW_ROTAR),
  IT_PANT("Mover",         nullptr, nullptr, SCREEN_SW_MOVER),
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
