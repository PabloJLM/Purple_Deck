#ifndef PURPLE_DECK_DRIVERS_SLOTS_H_
#define PURPLE_DECK_DRIVERS_SLOTS_H_
// ---------------------------------------------------------------------------
//  Drivers/slots.h - macros propias de los 3 menus Custom
//
//  Son 27 macros (3 menus x 9) que el usuario define sin recompilar: se editan
//  por la terminal serie (comando 'slot', ver 'slot help') o con tools.py.
//  Se guardan en la NVS, asi que no se pierden al subir 'data' (uploadfs).
//  Macro N (1..27): menu Custom ((N-1)/9 + 1), numero ((N-1)%9 + 1).
// ---------------------------------------------------------------------------
#include <Arduino.h>

struct Item;                                 // Estaticos/rejilla.h

#define SLOTS_POR_MENU 9
#define SLOTS_N        27

void     slotsIniciar();                     // carga de la NVS
bool     slotsComando(const String &linea);  // true si la linea era un comando 'slot'
void     slotEjecutar(uint8_t i);            // i = 0..SLOTS_N-1
uint32_t slotsVersion();                     // sube cada vez que algo cambia
bool     slotItem(uint8_t i, Item *out);     // false si la macro esta vacia

#endif  // PURPLE_DECK_DRIVERS_SLOTS_H_
