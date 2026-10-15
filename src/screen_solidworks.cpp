#include "Pantallas/screen_solidworks.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Iconos de 64x64 en LittleFS: /solidworks_1.png ... (en el orden de la tabla)
static const Item ITEMS[] = {
  IT_ACC("Reconstruir",     nullptr, "/solidworks_1.png", ACC_SW_RECONSTRUIR),
  IT_ACC("Ajustar\nvista",  nullptr, "/solidworks_2.png", ACC_SW_AJUSTAR),
  IT_ACC("Isometrica",      nullptr, "/solidworks_3.png", ACC_SW_ISOMETRICA),
  IT_ACC("Frontal",         nullptr, "/solidworks_4.png", ACC_SW_FRONTAL),
  IT_ACC("Superior",        nullptr, "/solidworks_5.png", ACC_SW_SUPERIOR),
  IT_ACC("Normal a",        nullptr, "/solidworks_6.png", ACC_SW_NORMAL_A),
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
