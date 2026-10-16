#include "Pantallas/screen_macros.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Iconos de 64x64 en LittleFS: /macros_1.png, /macros_2.png ... (en el orden
// de esta tabla). Siete por pagina; del 8 en adelante caen en la pagina 2.
static const Item ITEMS[] = {
  IT_ACC("Vol +",       nullptr, "/macros_1.png", ACC_VOL_UP),
  IT_ACC("Vol -",       nullptr, "/macros_2.png", ACC_VOL_DOWN),
  IT_ACC("Mute",        nullptr, "/macros_3.png", ACC_MUTE),
  IT_ACC("Play/Pausa",  nullptr, "/macros_4.png", ACC_PLAY_PAUSA),
  IT_ACC("Siguiente",   nullptr, "/macros_5.png", ACC_SIGUIENTE),
  IT_ACC("Anterior",    nullptr, "/macros_6.png", ACC_ANTERIOR),
  IT_ACC("Captura",     nullptr, "/macros_7.png", ACC_CAPTURA),
  IT_ACC("Escritorio",  nullptr, "/macros_8.png", ACC_ESCRITORIO),
  IT_ACC("Bloq. PC",    nullptr, "/macros_9.png", ACC_BLOQUEAR_PC),
  IT_ACC("Spotify",     nullptr, "/macros_10.png", ACC_ABRIR_SPOTIFY),
  IT_ACC("Navegador",   nullptr, "/macros_11.png", ACC_ABRIR_NAVEGADOR),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,           // 3x3: Atras abajo-izquierda, Siguiente abajo-derecha
  SCREEN_MENU
};

void screenMacrosEntrar() { rejillaEntrar(CFG); }
void screenMacrosLoop()   { rejillaLoop();      }
void screenMacrosSalir()  { rejillaSalir();     }
