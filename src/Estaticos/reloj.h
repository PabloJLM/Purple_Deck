#ifndef PURPLE_DECK_ESTATICOS_RELOJ_H_
#define PURPLE_DECK_ESTATICOS_RELOJ_H_
// ---------------------------------------------------------------------------
//  Estaticos/reloj.h - los dos dibujos de reloj (analogico y digital)
//  Dibujan sobre cualquier destino LovyanGFX: la pantalla o un sprite.
// ---------------------------------------------------------------------------
#include <time.h>
#include "Drivers/display.h"

void relojAnalogico(lgfx::LGFXBase *g, int16_t cx, int16_t cy, int16_t r,
                    const struct tm &t, bool valido);

void relojDigital(lgfx::LGFXBase *g, int16_t x, int16_t y, int16_t w, int16_t h,
                  const struct tm &t, bool valido);

#endif  // PURPLE_DECK_ESTATICOS_RELOJ_H_
