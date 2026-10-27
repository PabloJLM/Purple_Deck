#include "Pantallas/screen_labview.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"
#include "Drivers/menus.h"

// Solo texto: los iconos son unicamente los del menu principal.
static const Item ITEMS[] = {
  IT_ACC("Ejecutar",           nullptr, nullptr, ACC_LV_EJECUTAR),
  IT_ACC("Detener",            nullptr, nullptr, ACC_LV_DETENER),
  IT_ACC("Panel /\nDiagrama",  nullptr, nullptr, ACC_LV_PANEL_DIAGRAMA),
  IT_ACC("Ayuda",              nullptr, nullptr, ACC_LV_AYUDA),
  IT_ACC("Cables\nrotos",      nullptr, nullptr, ACC_LV_CABLES_ROTOS),
  IT_ACC("Ordenar",            nullptr, nullptr, ACC_LV_ORDENAR),
};

const Item *screenLabviewItems(uint8_t *n) { *n = sizeof(ITEMS) / sizeof(ITEMS[0]); return ITEMS; }

void screenLabviewEntrar() { menuEntrar(M_LABVIEW); }
void screenLabviewLoop()   { menuLoop(M_LABVIEW);   }
void screenLabviewSalir()  { menuSalir();    }
