#ifndef PURPLE_DECK_DRIVERS_SLOTS_H_
#define PURPLE_DECK_DRIVERS_SLOTS_H_
// ---------------------------------------------------------------------------
//  Drivers/slots.h - espacios editables del menu principal
//
//  Son 3 botones que el usuario define sin recompilar: se editan por la
//  terminal serie (comando 'slot', ver 'slot help') o con editor_slots.py.
//  Se guardan en la NVS, asi que no se pierden al subir 'data' (uploadfs).
// ---------------------------------------------------------------------------
#include <Arduino.h>

struct Item;                                 // Estaticos/rejilla.h

#define SLOTS_N 3

void     slotsIniciar();                     // carga de la NVS
bool     slotsComando(const String &linea);  // true si la linea era un comando 'slot'
void     slotEjecutar(uint8_t i);            // i = 0..SLOTS_N-1
uint32_t slotsVersion();                     // sube cada vez que algo cambia
uint8_t  slotsAgregarItems(Item *dest);      // agrega los slots no vacios; devuelve cuantos

#endif  // PURPLE_DECK_DRIVERS_SLOTS_H_
