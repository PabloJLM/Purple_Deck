#ifndef PURPLE_DECK_ESTATICOS_UI_H_
#define PURPLE_DECK_ESTATICOS_UI_H_
#include "Drivers/display.h"

void botonDibujar(lgfx::LGFXBase *g, int16_t x, int16_t y, int16_t w, int16_t h,
                  const char *texto, uint16_t relleno, uint16_t colorTexto,
                  const lgfx::IFont *fuente);
bool toqueEn(int16_t x, int16_t y, int16_t w, int16_t h);

#endif  // PURPLE_DECK_ESTATICOS_UI_H_
