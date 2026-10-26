#include "Pantallas/screen_sw_rotar.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Drivers/acciones.h"
#include "config.h"

#define AREA_Y   (CAB_ALTO + 4)
#define AREA_H   (ALTO - AREA_Y - GAP)
#define PASO_PX  30
#define GRADOS   15

static int32_t s_ax = 0, s_ay = 0;
static int16_t s_giroH = 0, s_giroV = 0;

static int16_t envolver(int16_t g)
{
  if (g > 180)   g -= 360;
  if (g <= -180) g += 360;
  return g;
}

static void dibujarArea()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  const int16_t cx = ANCHO / 2, cy = AREA_Y + AREA_H / 2;

  g->fillRoundRect(GAP, AREA_Y, ANCHO - 2 * GAP, AREA_H, RAD, t.superficie);
  g->drawRoundRect(GAP, AREA_Y, ANCHO - 2 * GAP, AREA_H, RAD, t.borde);
  g->drawCircle(cx, cy, 60, t.borde);
  g->drawFastHLine(cx - 40, cy, 80, t.borde);
  g->drawFastVLine(cx, cy - 40, 80, t.borde);

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);
  g->setTextDatum(top_center);
  g->setTextColor(t.textoTenue);
  g->drawString("Arrastra para rotar la pieza", cx, AREA_Y + 10);
}

static void dibujarGiro(bool empujar = true)
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  char b[32];
  snprintf(b, sizeof(b), "H %+d   V %+d", s_giroH, s_giroV);
  g->setTextSize(1.0f);
  g->setFont(&fonts::Font4);
  g->setTextDatum(bottom_center);
  g->setTextColor(t.acento, t.superficie);
  g->setTextPadding(260);
  g->drawString(b, ANCHO / 2, AREA_Y + AREA_H - 10);
  g->setTextPadding(0);
  if (empujar) pantallaEmpujar(ANCHO / 2 - 130, AREA_Y + AREA_H - 40, 260, 34);
}

static bool paso(Accion a, int16_t &giro, int16_t delta)
{
  accionEjecutar(a);
  giro = envolver(giro + delta);
  return true;
}

void screenSwRotarEntrar()
{
  s_giroH = s_giroV = 0;
  pantalla()->fillScreen(T().fondo);
  cabeceraDibujar("Rotar pieza", "< Salir");
  dibujarArea();
  dibujarGiro(false);
  pantallaEmpujarTodo();
}

void screenSwRotarLoop()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_SOLIDWORKS); return; }

  if (toqueNuevo) { s_ax = toqueX; s_ay = toqueY; }
  if (!toqueActivo || !toquePropio || toqueY0 < AREA_Y) return;

  bool cambio = false;
  while (toqueX - s_ax >= PASO_PX) { s_ax += PASO_PX; cambio = paso(ACC_SW_ROT_DER, s_giroH, +GRADOS); }
  while (s_ax - toqueX >= PASO_PX) { s_ax -= PASO_PX; cambio = paso(ACC_SW_ROT_IZQ, s_giroH, -GRADOS); }
  while (s_ay - toqueY >= PASO_PX) { s_ay -= PASO_PX; cambio = paso(ACC_SW_ROT_ARR, s_giroV, +GRADOS); }
  while (toqueY - s_ay >= PASO_PX) { s_ay += PASO_PX; cambio = paso(ACC_SW_ROT_ABA, s_giroV, -GRADOS); }

  if (cambio) dibujarGiro();
}

void screenSwRotarSalir() {}
