#include "Estaticos/reloj.h"
#include "Estaticos/temas.h"
#include "Drivers/tiempo.h"
#include <math.h>

// Manecilla como triangulo: base de ancho 'grosor' en el centro y punta en el
// extremo. Solo usa fillTriangle, asi no dependemos de drawWideLine.
static void manecilla(lgfx::LGFXBase *g, int16_t cx, int16_t cy,
                      float ang, float largo, float grosor, uint16_t color)
{
  float px = cosf(ang), py = sinf(ang);
  float nx = -py * grosor * 0.5f, ny = px * grosor * 0.5f;
  float colaX = cx - px * grosor, colaY = cy - py * grosor;

  g->fillTriangle(colaX + nx, colaY + ny,
                  colaX - nx, colaY - ny,
                  cx + px * largo, cy + py * largo, color);
}

// ---------------------------------------------------------------------------
//  Analogico: carátula tipo la referencia clara, pero en los tonos del tema.
// ---------------------------------------------------------------------------
void relojAnalogico(lgfx::LGFXBase *g, int16_t cx, int16_t cy, int16_t r,
                    const struct tm &t, bool valido)
{
  const Tema &tm_ = T();
  g->setTextSize(1.0f);

  g->fillCircle(cx, cy, r, tm_.superficieAlt);
  g->drawCircle(cx, cy, r, tm_.borde);
  g->drawCircle(cx, cy, r - 1, tm_.borde);

  // marcas de los minutos
  for (uint8_t i = 0; i < 60; i++) {
    float a = (i * 6.0f - 90.0f) * DEG_TO_RAD;
    bool  mayor = (i % 5 == 0);
    int16_t r0 = r - (mayor ? 10 : 5);
    int16_t x0 = cx + cosf(a) * r0,  y0 = cy + sinf(a) * r0;
    int16_t x1 = cx + cosf(a) * (r - 3), y1 = cy + sinf(a) * (r - 3);
    g->drawLine(x0, y0, x1, y1, mayor ? tm_.acento : tm_.borde);
  }

  // numeros romanos en las cuatro posiciones principales
  const char *rom[4] = { "XII", "III", "VI", "IX" };
  const int16_t ang[4] = { -90, 0, 90, 180 };
  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_center);
  g->setTextColor(tm_.texto);
  for (uint8_t i = 0; i < 4; i++) {
    float a = ang[i] * DEG_TO_RAD;
    g->drawString(rom[i], cx + cosf(a) * (r - 22), cy + sinf(a) * (r - 22));
  }

  if (!valido) {
    g->setTextColor(tm_.textoTenue);
    g->drawString("sin hora", cx, cy + r / 2);
    return;
  }

  float aH = ((t.tm_hour % 12) * 30.0f + t.tm_min * 0.5f - 90.0f) * DEG_TO_RAD;
  float aM = (t.tm_min * 6.0f + t.tm_sec * 0.1f - 90.0f) * DEG_TO_RAD;
  float aS = (t.tm_sec * 6.0f - 90.0f) * DEG_TO_RAD;

  // manecillas: hora y minuto gruesas, segundero fino dorado
  manecilla(g, cx, cy, aH, r * 0.50f, 8.0f, tm_.texto);
  manecilla(g, cx, cy, aM, r * 0.74f, 5.0f, tm_.texto);
  manecilla(g, cx, cy, aS, r * 0.84f, 3.0f, tm_.acento);

  g->fillCircle(cx, cy, 5, tm_.acento);
  g->fillCircle(cx, cy, 2, tm_.superficieAlt);
}

// ---------------------------------------------------------------------------
//  Digital: HH arriba y MM abajo con la fuente de 7 segmentos, segundos y
//  fecha en dorado, como la referencia oscura.
// ---------------------------------------------------------------------------
void relojDigital(lgfx::LGFXBase *g, int16_t x, int16_t y, int16_t w, int16_t h,
                  const struct tm &t, bool valido)
{
  const Tema &tm_ = T();
  const int16_t cx = x + w / 2;

  char hh[4], mm[4], ss[4];
  if (valido) {
    snprintf(hh, sizeof(hh), "%02d", t.tm_hour);
    snprintf(mm, sizeof(mm), "%02d", t.tm_min);
    snprintf(ss, sizeof(ss), "%02d", t.tm_sec);
  } else {
    strcpy(hh, "--"); strcpy(mm, "--"); strcpy(ss, "--");
  }

  g->setTextDatum(middle_center);
  g->setFont(&fonts::Font7);
  g->setTextSize(1.25f);

  g->setTextColor(tm_.texto);
  g->drawString(hh, cx, y + (int16_t)(h * 0.26f));
  g->drawString(mm, cx, y + (int16_t)(h * 0.56f));

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);

  // segundos en una pastilla dorada a la derecha
  const int16_t pw = 34, ph = 20;
  const int16_t px = x + w - pw - 14;
  const int16_t py = y + (int16_t)(h * 0.56f) + 22;
  g->fillRoundRect(px, py, pw, ph, ph / 2, tm_.acento);
  g->setTextColor(tm_.textoSobreAcento);
  g->drawString(ss, px + pw / 2, py + ph / 2);
}
