#include "Pantallas/screen_pomodoro.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Estaticos/ui.h"
#include "Drivers/display.h"
#include "Drivers/temporizadores.h"
#include "config.h"

#define ZONA_Y (CAB_ALTO + 4)
#define ZONA_H (ALTO - ZONA_Y)

#define ANILLO_CX 130
#define ANILLO_CY 130
#define ANILLO_RE 118
#define ANILLO_RI 100

#define RX 270
#define RW 200
#define PRINCIPAL_Y 14
#define PRINCIPAL_H 66
#define FILA_Y      92
#define FILA_H      52
#define FILA_W      96
#define AJUSTE_Y    156
#define AJUSTE_H    48
#define AJUSTE_W    56
#define PUNTOS_Y    232

static LGFX_Sprite s_sp(&lcd);
static bool     s_spOk  = false;
static uint32_t s_firma = 0xFFFFFFFF;

static uint32_t firma()
{
  return pomoRestanteS() | ((uint32_t)pomoFase() << 13) | ((uint32_t)pomoCorriendo() << 15) |
         ((uint32_t)pomoCiclo() << 16) | ((uint32_t)pomoIniciado() << 20) |
         ((uint32_t)pomoDuracionMin() << 21);
}

static const char *nombreFase()
{
  switch (pomoFase()) {
    case POMO_CORTO: return "Descanso";
    case POMO_LARGO: return "Descanso largo";
    default:         return "Foco";
  }
}

static void dibujar()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = s_spOk ? (lgfx::LGFXBase *)&s_sp : (lgfx::LGFXBase *)&lcd;
  const int16_t oy = s_spOk ? 0 : ZONA_Y;
  const uint16_t color = (pomoFase() == POMO_FOCO) ? t.acento : t.acentoAlt;
  char b[24];

  g->fillRect(0, oy, ANCHO, ZONA_H, t.fondo);

  const uint32_t total = pomoTotalS();
  const uint32_t resto = pomoRestanteS();
  const float frac = total ? (float)resto / (float)total : 0.0f;

  const int16_t cy = oy + ANILLO_CY;
  g->fillArc(ANILLO_CX, cy, ANILLO_RE, ANILLO_RI, 0.0f, 360.0f, t.superficieAlt);
  if (frac >= 0.999f)    g->fillArc(ANILLO_CX, cy, ANILLO_RE, ANILLO_RI, 0.0f, 360.0f, color);
  else if (frac > 0.0f)  g->fillArc(ANILLO_CX, cy, ANILLO_RE, ANILLO_RI, 270.0f, 270.0f + 360.0f * frac, color);

  g->setTextDatum(middle_center);
  g->setTextSize(1.0f);

  snprintf(b, sizeof(b), "Ciclo %u/%u", pomoCiclo(), POMO_CICLOS);
  g->setFont(&fonts::Font2);
  g->setTextColor(t.textoTenue);
  g->drawString(b, ANILLO_CX, cy - 52);

  snprintf(b, sizeof(b), "%02lu:%02lu", (unsigned long)(resto / 60), (unsigned long)(resto % 60));
  g->setFont(&fonts::Font7);
  g->setTextColor(t.texto);
  g->drawString(b, ANILLO_CX, cy);

  g->setFont(&fonts::Font4);
  g->setTextColor(color);
  g->drawString(nombreFase(), ANILLO_CX, cy + 50);

  const char *principal = pomoCorriendo() ? "Pausa" : (pomoIniciado() ? "Continuar" : "Iniciar");
  botonDibujar(g, RX, oy + PRINCIPAL_Y, RW, PRINCIPAL_H, principal, t.acento, t.textoSobreAcento, &fonts::Font4);
  botonDibujar(g, RX, oy + FILA_Y, FILA_W, FILA_H, "Saltar", t.superficie, t.texto, &fonts::Font4);
  botonDibujar(g, RX + RW - FILA_W, oy + FILA_Y, FILA_W, FILA_H, "Reiniciar", t.superficie, t.texto, &fonts::Font2);

  if (!pomoIniciado()) {
    botonDibujar(g, RX, oy + AJUSTE_Y, AJUSTE_W, AJUSTE_H, "-", t.superficie, t.texto, &fonts::Font4);
    botonDibujar(g, RX + RW - AJUSTE_W, oy + AJUSTE_Y, AJUSTE_W, AJUSTE_H, "+", t.superficie, t.texto, &fonts::Font4);
    snprintf(b, sizeof(b), "%u min", pomoDuracionMin());
    g->setFont(&fonts::Font4);
    g->setTextDatum(middle_center);
    g->setTextColor(t.texto);
    g->drawString(b, RX + RW / 2, oy + AJUSTE_Y + AJUSTE_H / 2);
  }

  const uint8_t hechos = (pomoFase() == POMO_FOCO) ? pomoCiclo() - 1 : pomoCiclo();
  for (uint8_t i = 0; i < POMO_CICLOS; i++) {
    const int16_t px = RX + RW / 2 + (i - (POMO_CICLOS - 1) / 2.0f) * 28;
    if (i < hechos) g->fillCircle(px, oy + PUNTOS_Y, 7, t.acento);
    else            g->drawCircle(px, oy + PUNTOS_Y, 7, t.borde);
  }

  if (s_spOk) s_sp.pushSprite(0, ZONA_Y);
}

void screenPomodoroEntrar()
{
  s_sp.setColorDepth(16);
  s_sp.setPsram(psramFound());
  s_spOk = s_sp.createSprite(ANCHO, ZONA_H);

  lcd.fillScreen(T().fondo);
  cabeceraDibujar("Pomodoro", "< Menu");
  s_firma = firma();
  dibujar();
}

void screenPomodoroLoop()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_MENU); return; }

  if (toqueEn(RX, ZONA_Y + PRINCIPAL_Y, RW, PRINCIPAL_H)) pomoIniciarPausar();
  else if (toqueEn(RX, ZONA_Y + FILA_Y, FILA_W, FILA_H)) pomoSaltar();
  else if (toqueEn(RX + RW - FILA_W, ZONA_Y + FILA_Y, FILA_W, FILA_H)) pomoReiniciar();
  else if (!pomoIniciado()) {
    if (toqueEn(RX, ZONA_Y + AJUSTE_Y, AJUSTE_W, AJUSTE_H)) pomoAjustar(-1);
    else if (toqueEn(RX + RW - AJUSTE_W, ZONA_Y + AJUSTE_Y, AJUSTE_W, AJUSTE_H)) pomoAjustar(+1);
  }

  const uint32_t f = firma();
  if (f != s_firma) { s_firma = f; dibujar(); }
}

void screenPomodoroSalir()
{
  if (s_spOk) { s_sp.deleteSprite(); s_spOk = false; }
}
