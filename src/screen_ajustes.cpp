#include "Pantallas/screen_ajustes.h"
#include "Pantallas/screen_idle.h"
#include "Estaticos/rejilla.h"
#include "Estaticos/temas.h"
#include "Drivers/acciones.h"
#include "config.h"

// ---------------------------------------------------------------------------
//  Textos que muestran el valor actual de cada ajuste debajo del nombre.
//  El motor los vuelve a pedir cada vez que redibuja el boton.
// ---------------------------------------------------------------------------
static const char *subBrillo()
{
  static char b[12];
  snprintf(b, sizeof(b), "%u%%", (unsigned)(brilloNormal * 100UL / 255UL));
  return b;
}

static const char *subReposo()
{
  static char b[12];
  snprintf(b, sizeof(b), "%lu s", (unsigned long)(idleTimeoutMs / 1000UL));
  return b;
}

static const char *subReloj()
{
  return relojModo() == RELOJ_ANALOGICO ? "Analogico" : "Digital";
}

static const char *subTema()
{
  return T().nombre;
}

// ---------------------------------------------------------------------------
static const Item ITEMS[] = {
  IT_ACC_DIN("Brillo -",  nullptr, ACC_AJ_BRILLO_MENOS, subBrillo),
  IT_ACC_DIN("Brillo +",  nullptr, ACC_AJ_BRILLO_MAS,   subBrillo),
  IT_ACC_DIN("Reposo",    nullptr, ACC_AJ_REPOSO,       subReposo),
  IT_ACC_DIN("Reloj",     nullptr, ACC_AJ_RELOJ,        subReloj),
  IT_ACC_DIN("Tema",      nullptr, ACC_AJ_TEMA,         subTema),
  IT_ACC("Reiniciar",     "Reinicia el deck", nullptr,  ACC_AJ_REINICIAR),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,           // 3x3: Atras abajo-izquierda
  SCREEN_MENU
};

void screenAjustesEntrar() { rejillaEntrar(CFG); }
void screenAjustesLoop()   { rejillaLoop();      }
void screenAjustesSalir()  { rejillaSalir();     }
