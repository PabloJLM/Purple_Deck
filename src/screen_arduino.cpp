#include "Pantallas/screen_arduino.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Iconos de 64x64 en LittleFS: /arduino_1.png ... (en el orden de la tabla)
static const Item ITEMS[] = {
  IT_ACC("Verificar",         nullptr, "/arduino_1.png", ACC_ARD_VERIFICAR),
  IT_ACC("Subir",             nullptr, "/arduino_2.png", ACC_ARD_SUBIR),
  IT_ACC("Monitor\nserie",    nullptr, "/arduino_3.png", ACC_ARD_MONITOR),
  IT_ACC("Plotter",           nullptr, "/arduino_4.png", ACC_ARD_PLOTTER),
  IT_ACC("Guardar",           nullptr, "/arduino_5.png", ACC_ARD_GUARDAR),
  IT_ACC("Librerias",         nullptr, "/arduino_6.png", ACC_ARD_LIBRERIAS),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenArduinoEntrar() { rejillaEntrar(CFG); }
void screenArduinoLoop()   { rejillaLoop();      }
void screenArduinoSalir()  { rejillaSalir();     }
