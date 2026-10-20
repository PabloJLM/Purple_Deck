#include "Estaticos/ui.h"
#include "Estaticos/temas.h"
#include "config.h"

void botonDibujar(lgfx::LGFXBase *g, int16_t x, int16_t y, int16_t w, int16_t h,
                  const char *texto, uint16_t relleno, uint16_t colorTexto,
                  const lgfx::IFont *fuente)
{
  g->fillRoundRect(x, y, w, h, RAD, relleno);
  g->drawRoundRect(x, y, w, h, RAD, T().borde);
  g->setTextSize(1.0f);
  g->setFont(fuente);
  g->setTextDatum(middle_center);
  g->setTextColor(colorTexto);
  g->drawString(texto, x + w / 2, y + h / 2);
}

bool toqueEn(int16_t x, int16_t y, int16_t w, int16_t h)
{
  return toqueNuevo && toqueX >= x && toqueX < x + w && toqueY >= y && toqueY < y + h;
}
