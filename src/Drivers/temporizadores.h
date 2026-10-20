#ifndef PURPLE_DECK_DRIVERS_TEMPORIZADORES_H_
#define PURPLE_DECK_DRIVERS_TEMPORIZADORES_H_
#include <Arduino.h>
#include "config.h"

enum PomoFase : uint8_t { POMO_FOCO = 0, POMO_CORTO, POMO_LARGO };

struct Alarma {
  uint8_t hora;
  uint8_t minuto;
  bool    activa;
};

void temporizadoresIniciar();
bool temporizadoresLoop();

void     pomoIniciarPausar();
void     pomoReiniciar();
void     pomoSaltar();
void     pomoAjustar(int8_t deltaMin);
bool     pomoCorriendo();
bool     pomoIniciado();
PomoFase pomoFase();
uint8_t  pomoCiclo();
uint8_t  pomoDuracionMin();
uint32_t pomoRestanteS();
uint32_t pomoTotalS();

void     cronoIniciarPausar();
void     cronoReiniciar();
void     cronoVuelta();
bool     cronoCorriendo();
uint32_t cronoMs();
uint8_t  cronoVueltas();
uint32_t cronoVueltaMs(uint8_t i);

Alarma &alarma(uint8_t i);
void    alarmasGuardar();
void    alarmaSilenciar();

float tempChip();

#endif  // PURPLE_DECK_DRIVERS_TEMPORIZADORES_H_
