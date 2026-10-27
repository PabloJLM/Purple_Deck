#include "Pantallas/screen_git.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"

// Solo texto: los iconos son unicamente los del menu principal.
// Siete por pagina; del 8 en adelante caen en la pagina 2.
static const Item ITEMS[] = {
  IT_ACC("Status",           nullptr, nullptr, ACC_GIT_STATUS),
  IT_ACC("Pull",             nullptr, nullptr, ACC_GIT_PULL),
  IT_ACC("Push",             nullptr, nullptr, ACC_GIT_PUSH),
  IT_ACC("Commit",           nullptr, nullptr, ACC_GIT_COMMIT),
  IT_ACC("Add all",          nullptr, nullptr, ACC_GIT_ADD_ALL),
  IT_ACC("Terminal",         nullptr, nullptr, ACC_VS_TERMINAL),
  IT_ACC("Paleta",           nullptr, nullptr, ACC_VS_PALETA),
  IT_ACC("Buscar",           nullptr, nullptr, ACC_VS_BUSCAR),
  IT_ACC("Guardar\ntodo",    nullptr, nullptr, ACC_VS_GUARDAR_TODO),
  IT_ACC("Abrir\ncarpeta",   nullptr, nullptr, ACC_VS_ABRIR_CARPETA),
  IT_ACC("Terminal\nexterna", nullptr, nullptr, ACC_VS_TERMINAL_EXT),
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
