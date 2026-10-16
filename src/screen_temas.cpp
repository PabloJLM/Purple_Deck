#include "Pantallas/screen_temas.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "config.h"

#define LISTA_Y  (CAB_ALTO + 4)
#define LISTA_H  (ALTO - LISTA_Y)
#define FILA_H   64
#define VISIBLES (LISTA_H / FILA_H)
#define PASO_PX  (FILA_H / 2)
#define FILA_W   (ANCHO - 2 * GAP - 12)

static LGFX_Sprite s_sp(&lcd);
static bool    s_spOk = false;
static int16_t s_primera = 0;
static int16_t s_primeraIni = 0;

static int16_t maxPrimera()
{
  return NUM_TEMAS > VISIBLES ? NUM_TEMAS - VISIBLES : 0;
}

static void dibujarLista()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = s_spOk ? (lgfx::LGFXBase *)&s_sp : (lgfx::LGFXBase *)&lcd;
  const int16_t oy = s_spOk ? 0 : LISTA_Y;

  g->fillRect(0, oy, ANCHO, LISTA_H, t.fondo);
  g->setTextSize(1.0f);

  for (int16_t f = 0; f < VISIBLES; f++) {
    const int16_t i = s_primera + f;
    if (i >= NUM_TEMAS) break;

    const Tema &e = TEMAS[i];
    const bool activo = (i == temaIndice());
    const int16_t y = oy + f * FILA_H;

    g->fillRoundRect(GAP, y + 3, FILA_W, FILA_H - 6, RAD, activo ? t.superficieAlt : t.superficie);
    g->drawRoundRect(GAP, y + 3, FILA_W, FILA_H - 6, RAD, activo ? t.acento : t.borde);

    const uint16_t muestra[4] = { e.fondo, e.primario, e.acento, e.texto };
    for (uint8_t k = 0; k < 4; k++) {
      const int16_t x = GAP + 14 + k * 30;
      g->fillRoundRect(x, y + 18, 24, 28, 6, muestra[k]);
      g->drawRoundRect(x, y + 18, 24, 28, 6, t.borde);
    }

    g->setFont(&fonts::Font4);
    g->setTextDatum(middle_left);
    g->setTextColor(t.texto);
    g->drawString(e.nombre, GAP + 14 + 4 * 30 + 10, y + FILA_H / 2);

    if (activo) {
      g->setFont(&fonts::Font2);
      g->setTextDatum(middle_right);
      g->setTextColor(t.acento);
      g->drawString("en uso", GAP + FILA_W - 14, y + FILA_H / 2);
    }
  }

  if (maxPrimera() > 0) {
    const int16_t barH = LISTA_H * VISIBLES / NUM_TEMAS;
    const int16_t barY = oy + (LISTA_H - barH) * s_primera / maxPrimera();
    g->fillRect(ANCHO - 8, oy, 4, LISTA_H, t.superficie);
    g->fillRect(ANCHO - 8, barY, 4, barH, t.acento);
  }

  if (s_spOk) s_sp.pushSprite(0, LISTA_Y);
}

static void dibujarTodo()
{
  lcd.fillScreen(T().fondo);
  cabeceraDibujar("Tema", "< Ajustes");
  dibujarLista();
}

void screenTemasEntrar()
{
  s_primera = temaIndice() < maxPrimera() ? temaIndice() : maxPrimera();

  s_sp.setColorDepth(16);
  s_sp.setPsram(psramFound());
  s_spOk = s_sp.createSprite(ANCHO, LISTA_H);

  dibujarTodo();
}

void screenTemasLoop()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_AJUSTES); return; }

  if (toqueNuevo) s_primeraIni = s_primera;

  if (toqueActivo && toquePropio && toqueY0 >= LISTA_Y) {
    int16_t p = s_primeraIni - (int16_t)((toqueY - toqueY0) / PASO_PX);
    p = constrain(p, 0, maxPrimera());
    if (p != s_primera) { s_primera = p; dibujarLista(); }
  }

  if (fueToque() && toqueY0 >= LISTA_Y) {
    const int16_t i = s_primera + (toqueY0 - LISTA_Y) / FILA_H;
    if (i < NUM_TEMAS && i != temaIndice()) {
      temaUsar(i);
      dibujarTodo();
    }
  }
}

void screenTemasSalir()
{
  if (s_spOk) { s_sp.deleteSprite(); s_spOk = false; }
}
