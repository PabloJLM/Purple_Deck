#include "Pantallas/screen_candle.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Iconos de 64x64 en LittleFS: /candle_1.png ... (en el orden de la tabla)
// 7 por pagina: Atras abajo a la izquierda, Siguiente abajo a la derecha.
static const Item ITEMS[] = {
  IT_ACC("Enviar",          nullptr, "/candle_1.png", ACC_CN_ENVIAR),
  IT_ACC("Pausa",           nullptr, "/candle_2.png", ACC_CN_PAUSA),
  IT_ACC("Abortar",         nullptr, "/candle_3.png", ACC_CN_ABORTAR),
  IT_ACC("Reset",           nullptr, "/candle_4.png", ACC_CN_RESET),
  IT_ACC("Desbloquear",     nullptr, "/candle_5.png", ACC_CN_DESBLOQUEAR),
  IT_ACC("Ir a\nX0 Y0",     nullptr, "/candle_6.png", ACC_CN_HOME_XY),
  IT_PANT("Mover",          "X Y Z",  "/candle_7.png", SCREEN_CANDLE_MOVER),
  // pagina 2
  IT_ACC("Husillo\non/off", nullptr, "/candle_9.png", ACC_CN_HUSILLO),
  IT_ACC("Heightmap",       nullptr, "/candle_10.png", ACC_CN_HEIGHTMAP),
  IT_ACC("Probe Z",         nullptr, "/candle_11.png", ACC_CN_PROBE_Z),
  IT_ACC("Z+",              nullptr, "/candle_z_mas.png",   ACC_CN_Z_MAS),
  IT_ACC("Z-",              nullptr, "/candle_z_menos.png", ACC_CN_Z_MENOS),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenCandleEntrar() { rejillaEntrar(CFG); }
void screenCandleLoop()   { rejillaLoop();      }
void screenCandleSalir()  { rejillaSalir();     }

// --- Mover: jog en X, Y, Z (exactamente 7 items = una pagina) --------------
static const Item MOVER[] = {
  IT_ACC("X-",    nullptr, "/candle_x_menos.png", ACC_CN_X_MENOS),
  IT_ACC("X+",    nullptr, "/candle_x_mas.png",   ACC_CN_X_MAS),
  IT_ACC("Y-",    nullptr, "/candle_y_menos.png", ACC_CN_Y_MENOS),
  IT_ACC("Y+",    nullptr, "/candle_y_mas.png",   ACC_CN_Y_MAS),
  IT_ACC("Z-",    nullptr, "/candle_z_menos.png", ACC_CN_Z_MENOS),
  IT_ACC("Z+",    nullptr, "/candle_z_mas.png",   ACC_CN_Z_MAS),
  IT_ACC("Parar",  nullptr, "/candle_parar.png",  ACC_CN_PARAR),
};

static const RejillaCfg CFG_MOVER = {
  MOVER, sizeof(MOVER) / sizeof(MOVER[0]),
  3,
  true,
  SCREEN_CANDLE
};

void screenCandleMoverEntrar() { rejillaEntrar(CFG_MOVER); }
void screenCandleMoverLoop()   { rejillaLoop();            }
void screenCandleMoverSalir()  { rejillaSalir();           }
