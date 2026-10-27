#include "Pantallas/screen_macros.h"
#include "Estaticos/rejilla.h"
#include "Drivers/acciones.h"
#include "Drivers/menus.h"

// Solo texto: los iconos son unicamente los del menu principal.
// de esta tabla). Siete por pagina; del 8 en adelante caen en la pagina 2.
static const Item ITEMS[] = {
  IT_ACC("Vol +",       nullptr, nullptr, ACC_VOL_UP),
  IT_ACC("Vol -",       nullptr, nullptr, ACC_VOL_DOWN),
  IT_ACC("Mute",        nullptr, nullptr, ACC_MUTE),
  IT_ACC("Play/Pausa",  nullptr, nullptr, ACC_PLAY_PAUSA),
  IT_ACC("Siguiente",   nullptr, nullptr, ACC_SIGUIENTE),
  IT_ACC("Anterior",    nullptr, nullptr, ACC_ANTERIOR),
  IT_ACC("Captura",     nullptr, nullptr, ACC_CAPTURA),
  IT_ACC("Escritorio",  nullptr, nullptr, ACC_ESCRITORIO),
  IT_ACC("Bloq. PC",    nullptr, nullptr, ACC_BLOQUEAR_PC),
  IT_ACC("Spotify",     nullptr, nullptr, ACC_ABRIR_SPOTIFY),
  IT_ACC("Navegador",   nullptr, nullptr, ACC_ABRIR_NAVEGADOR),
};

const Item *screenMacrosItems(uint8_t *n) { *n = sizeof(ITEMS) / sizeof(ITEMS[0]); return ITEMS; }

void screenMacrosEntrar() { menuEntrar(M_MACROS); }
void screenMacrosLoop()   { menuLoop(M_MACROS);   }
void screenMacrosSalir()  { menuSalir();    }
