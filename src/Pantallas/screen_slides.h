#ifndef PURPLE_DECK_PANTALLAS_SCREEN_SLIDES_H_
#define PURPLE_DECK_PANTALLAS_SCREEN_SLIDES_H_
#include <Arduino.h>

void slidesIniciar();
void slidesEscanear();
uint8_t slidesCantidad();
const char *slidesOrigen();

void screenSlidesEntrar();
void screenSlidesLoop();
void screenSlidesSalir();
#endif  // PURPLE_DECK_PANTALLAS_SCREEN_SLIDES_H_
