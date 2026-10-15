#ifndef PURPLE_DECK_PANTALLAS_SCREEN_IDLE_H_
#define PURPLE_DECK_PANTALLAS_SCREEN_IDLE_H_
// ---------------------------------------------------------------------------
//  Pantallas/screen_idle.h - pantalla de bloqueo: reloj + notificaciones
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include "config.h"

void screenIdleEntrar();
void screenIdleLoop();
void screenIdleSalir();

RelojModo relojModo();              // analogico o digital
void      relojUsarModo(RelojModo m);
void      relojAlternarModo();      // tocar la tarjeta del reloj lo cambia

#endif  // PURPLE_DECK_PANTALLAS_SCREEN_IDLE_H_
