#include "Pantallas/screen_labview.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Iconos de 64x64 en LittleFS: /labview_1.png ... (en el orden de la tabla)
static const Item ITEMS[] = {
  IT_ACC("Ejecutar",           nullptr, "/labview_1.png", ACC_LV_EJECUTAR),
  IT_ACC("Detener",            nullptr, "/labview_2.png", ACC_LV_DETENER),
  IT_ACC("Panel /\nDiagrama",  nullptr, "/labview_3.png", ACC_LV_PANEL_DIAGRAMA),
  IT_ACC("Ayuda",              nullptr, "/labview_4.png", ACC_LV_AYUDA),
  IT_ACC("Cables\nrotos",      nullptr, "/labview_5.png", ACC_LV_CABLES_ROTOS),
  IT_ACC("Ordenar",            nullptr, "/labview_6.png", ACC_LV_ORDENAR),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenLabviewEntrar() { rejillaEntrar(CFG); }
void screenLabviewLoop()   { rejillaLoop();      }
void screenLabviewSalir()  { rejillaSalir();     }
