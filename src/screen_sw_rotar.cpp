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
  const int16_t cx = ANCHO / 2, cy = AREA_Y + AREA_H / 2;

  lcd.fillRoundRect(GAP, AREA_Y, ANCHO - 2 * GAP, AREA_H, RAD, t.superficie);
  lcd.drawRoundRect(GAP, AREA_Y, ANCHO - 2 * GAP, AREA_H, RAD, t.borde);
  lcd.drawCircle(cx, cy, 60, t.borde);
  lcd.drawFastHLine(cx - 40, cy, 80, t.borde);
  lcd.drawFastVLine(cx, cy - 40, 80, t.borde);

  lcd.setTextSize(1.0f);
  lcd.setFont(&fonts::Font2);
  lcd.setTextDatum(top_center);
  lcd.setTextColor(t.textoTenue);
  lcd.drawString("Arrastra para rotar la pieza", cx, AREA_Y + 10);
}

static void dibujarGiro()
{
  const Tema &t = T();
  char b[32];
  snprintf(b, sizeof(b), "H %+d   V %+d", s_giroH, s_giroV);
  lcd.setTextSize(1.0f);
  lcd.setFont(&fonts::Font4);
  lcd.setTextDatum(bottom_center);
  lcd.setTextColor(t.acento, t.superficie);
  lcd.setTextPadding(260);
  lcd.drawString(b, ANCHO / 2, AREA_Y + AREA_H - 10);
  lcd.setTextPadding(0);
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
  lcd.fillScreen(T().fondo);
  cabeceraDibujar("Rotar pieza", "< Salir");
  dibujarArea();
  dibujarGiro();
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
