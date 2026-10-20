#ifndef PURPLE_DECK_DRIVERS_AUDIO_H_
#define PURPLE_DECK_DRIVERS_AUDIO_H_
#include <Arduino.h>
#include "Drivers/notificaciones.h"

bool audioIniciar();
void audioReproducir(const char *ruta);
void audioNotificacion(NotifTipo tipo);

#endif  // PURPLE_DECK_DRIVERS_AUDIO_H_
