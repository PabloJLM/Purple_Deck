#include "Pantallas/screen_custom.h"
#include "Drivers/menus.h"

void screenCustomEntrar(uint8_t k) { menuEntrar((MenuId)(M_CUSTOM1 + k)); }
void screenCustomLoop(uint8_t k)   { menuLoop((MenuId)(M_CUSTOM1 + k));   }
void screenCustomSalir()           { menuSalir();                         }
