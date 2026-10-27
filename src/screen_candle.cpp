#include "Pantallas/screen_candle.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"
#include "Drivers/menus.h"

// Solo texto: los iconos son unicamente los del menu principal.
// 7 por pagina: Atras abajo a la izquierda, Siguiente abajo a la derecha.
static const Item ITEMS[] = {
  IT_ACC("Enviar",          nullptr, nullptr, ACC_CN_ENVIAR),
  IT_ACC("Pausa",           nullptr, nullptr, ACC_CN_PAUSA),
  IT_ACC("Abortar",         nullptr, nullptr, ACC_CN_ABORTAR),
  IT_ACC("Reset",           nullptr, nullptr, ACC_CN_RESET),
  IT_ACC("Desbloquear",     nullptr, nullptr, ACC_CN_DESBLOQUEAR),
  IT_PANT("Mover",          "X Y Z",  nullptr, SCREEN_CANDLE_MOVER),
  // pagina 2
  IT_ACC("Husillo\non/off", nullptr, nullptr, ACC_CN_HUSILLO),
  IT_ACC("Heightmap",       nullptr, nullptr, ACC_CN_HEIGHTMAP),
  IT_ACC("Probe Z",         nullptr, nullptr, ACC_CN_PROBE_Z),
  IT_ACC("Zero XY",         nullptr, nullptr, ACC_CN_ZERO_XY),
  IT_ACC("Zero Z",          nullptr, nullptr, ACC_CN_ZERO_Z),
  IT_ACC("Ir a\nX0 Y0",     nullptr, nullptr,  ACC_CN_IR_XY0),
};


// --- Mover: jog en X, Y, Z (exactamente 7 items = una pagina) --------------
static const Item MOVER[] = {
  IT_ACC("X-",    nullptr, nullptr, ACC_CN_X_MENOS),
  IT_ACC("X+",    nullptr, nullptr,   ACC_CN_X_MAS),
  IT_ACC("Y-",    nullptr, nullptr, ACC_CN_Y_MENOS),
  IT_ACC("Y+",    nullptr, nullptr,   ACC_CN_Y_MAS),
  IT_ACC("Z-",    nullptr, nullptr, ACC_CN_Z_MENOS),
  IT_ACC("Z+",    nullptr, nullptr,   ACC_CN_Z_MAS),
  IT_ACC("Parar",  nullptr, nullptr,  ACC_CN_PARAR),
};

const Item *screenCandleItems(uint8_t *n)      { *n = sizeof(ITEMS) / sizeof(ITEMS[0]); return ITEMS; }
const Item *screenCandleMoverItems(uint8_t *n) { *n = sizeof(MOVER) / sizeof(MOVER[0]); return MOVER; }

void screenCandleEntrar() { menuEntrar(M_CANDLE); }
void screenCandleLoop()   { menuLoop(M_CANDLE);   }
void screenCandleSalir()  { menuSalir();          }

void screenCandleMoverEntrar() { menuEntrar(M_CANDLE_MOVER); }
void screenCandleMoverLoop()   { menuLoop(M_CANDLE_MOVER);   }
void screenCandleMoverSalir()  { menuSalir();                }
