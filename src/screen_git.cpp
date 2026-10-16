#include "Pantallas/screen_git.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Iconos de 64x64 en LittleFS: /git_1.png ... (en el orden de la tabla)
// Siete por pagina; del 8 en adelante caen en la pagina 2.
static const Item ITEMS[] = {
  IT_ACC("Status",           nullptr, "/git_1.png", ACC_GIT_STATUS),
  IT_ACC("Pull",             nullptr, "/git_2.png", ACC_GIT_PULL),
  IT_ACC("Push",             nullptr, "/git_3.png", ACC_GIT_PUSH),
  IT_ACC("Commit",           nullptr, "/git_4.png", ACC_GIT_COMMIT),
  IT_ACC("Add all",          nullptr, "/git_5.png", ACC_GIT_ADD_ALL),
  IT_ACC("Terminal",         nullptr, "/git_6.png", ACC_VS_TERMINAL),
  IT_ACC("Paleta",           nullptr, "/git_7.png", ACC_VS_PALETA),
  IT_ACC("Buscar",           nullptr, "/git_8.png", ACC_VS_BUSCAR),
  IT_ACC("Guardar\ntodo",    nullptr, "/git_9.png", ACC_VS_GUARDAR_TODO),
  IT_ACC("Abrir\ncarpeta",   nullptr, "/git_10.png", ACC_VS_ABRIR_CARPETA),
  IT_ACC("Terminal\nexterna", nullptr, "/git_11.png", ACC_VS_TERMINAL_EXT),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenGitEntrar() { rejillaEntrar(CFG); }
void screenGitLoop()   { rejillaLoop();      }
void screenGitSalir()  { rejillaSalir();     }
