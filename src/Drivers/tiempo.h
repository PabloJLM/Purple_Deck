#ifndef PURPLE_DECK_DRIVERS_TIEMPO_H_
#define PURPLE_DECK_DRIVERS_TIEMPO_H_
// ---------------------------------------------------------------------------
//  Drivers/tiempo.h - WiFi + NTP, no bloqueante
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <time.h>

void tiempoIniciar();              // arranca el WiFi (no espera)
void tiempoLoop();                 // llamar en cada vuelta del loop
bool wifiConectado();
bool tiempoSincronizado();         // ya hay hora valida por NTP
bool tiempoObtener(struct tm &t);  // false mientras no haya hora

// Nombres en ASCII (las fuentes del display no llevan acentos)
extern const char *DIA_CORTO[7];
extern const char *DIA_LARGO[7];
extern const char *MES_CORTO[12];

#endif  // PURPLE_DECK_DRIVERS_TIEMPO_H_
