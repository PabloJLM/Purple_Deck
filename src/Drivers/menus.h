#ifndef PURPLE_DECK_DRIVERS_MENUS_H_
#define PURPLE_DECK_DRIVERS_MENUS_H_
// ---------------------------------------------------------------------------
//  Drivers/menus.h - organizacion de todos los menus
//
//  Cada pantalla de rejilla declara sus botones "de fabrica" (screen_*.cpp);
//  este modulo los junta en un registro con una clave estable por boton y
//  decide en que menu va cada uno y en que orden segun la distribucion
//  guardada en la NVS. La distribucion se cambia por la terminal serie
//  (comando 'menu', ver 'menu help') o con tools.py, sin recompilar.
//
//  Se puede reordenar, mover un boton a otro menu u ocultarlo. Lo que no tiene
//  distribucion guardada queda como viene de fabrica; un boton nuevo del
//  firmware aparece en su menu de origen.
// ---------------------------------------------------------------------------
#include <Arduino.h>

struct Item;                                 // Estaticos/rejilla.h

enum MenuId : uint8_t {
  M_PRINCIPAL = 0,
  M_MACROS, M_SOLIDWORKS, M_LABVIEW, M_GIT, M_BLENDER,
  M_CANDLE, M_CANDLE_MOVER,
  M_CUSTOM1, M_CUSTOM2, M_CUSTOM3,
  MENUS_N
};

void        menusIniciar();                  // carga nombres y distribucion (tras slotsIniciar)
bool        menusComando(const String &linea);
const char *menuNombreCustom(uint8_t k);     // k = 0..2

// Las pantallas de rejilla se reducen a esto:
void menuEntrar(MenuId m);
void menuLoop(MenuId m);
void menuSalir();

// Botones de fabrica de cada menu (los define cada screen_*.cpp)
const Item *screenMenuItems(uint8_t *n);
const Item *screenMacrosItems(uint8_t *n);
const Item *screenSolidworksItems(uint8_t *n);
const Item *screenLabviewItems(uint8_t *n);
const Item *screenGitItems(uint8_t *n);
const Item *screenBlenderItems(uint8_t *n);
const Item *screenCandleItems(uint8_t *n);
const Item *screenCandleMoverItems(uint8_t *n);

#endif  // PURPLE_DECK_DRIVERS_MENUS_H_
