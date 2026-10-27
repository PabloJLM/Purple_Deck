#include "Pantallas/screen_sw_rotar.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Drivers/acciones.h"
#include "config.h"

// ---------------------------------------------------------------------------
//  Area tactil de SolidWorks. Se arrastra el dedo y por cada PASO_PX de
//  recorrido se manda una pulsacion de flecha:
//    Rotar : flechas              -> giran el modelo (en pasos de 15 grados)
//    Mover : Ctrl + flechas       -> desplazan (pan) el modelo en la vista
// ---------------------------------------------------------------------------
#define AREA_Y   (CAB_ALTO + 4)
#define AREA_H   (ALTO - AREA_Y - GAP)
#define PASO_PX  30
#define GRADOS   15

struct Modo {
  const char *titulo;
  const char *ayuda;
  Accion      izq, der, arr, aba;
  bool        grados;      // true: muestra grados y los envuelve; false: cuenta pasos
};

static const Modo ROTAR = { "Rotar", "Arrastra para rotar",
                            ACC_SW_ROT_IZQ, ACC_SW_ROT_DER, ACC_SW_ROT_ARR, ACC_SW_ROT_ABA, true };
static const Modo MOVER = { "Mover", "Arrastra para mover",
                            ACC_SW_MOV_IZQ, ACC_SW_MOV_DER, ACC_SW_MOV_ARR, ACC_SW_MOV_ABA, false };

static const Modo *s_modo = &ROTAR;
static Screen      s_volver = SCREEN_SOLIDWORKS;

static int32_t s_ax = 0, s_ay = 0;
static int16_t s_h = 0, s_v = 0;

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
  g->drawString(s_modo->ayuda, cx, AREA_Y + 10);
}

static void dibujarContador(bool empujar = true)
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  char b[32];
  if (s_modo->grados) snprintf(b, sizeof(b), "H %+d   V %+d", s_h, s_v);
  else                snprintf(b, sizeof(b), "X %+d   Y %+d", s_h, s_v);
  g->setTextSize(1.0f);
  g->setFont(&fonts::Font4);
  g->setTextDatum(bottom_center);
  g->setTextColor(t.acento, t.superficie);
  g->setTextPadding(260);
  g->drawString(b, ANCHO / 2, AREA_Y + AREA_H - 10);
  g->setTextPadding(0);
  if (empujar) pantallaEmpujar(ANCHO / 2 - 130, AREA_Y + AREA_H - 40, 260, 34);
}

// dir: +1 / -1; eje: 0 = horizontal, 1 = vertical
static bool paso(Accion a, int16_t &valor, int16_t delta)
{
  accionEjecutar(a);
  valor += delta;
  if (s_modo->grados) valor = envolver(valor);
  return true;
}

static void entrarModo(const Modo *m)
{
  s_modo = m;
  s_h = s_v = 0;
  pantalla()->fillScreen(T().fondo);
  cabeceraDibujar(m->titulo, "< Salir");
  dibujarArea();
  dibujarContador(false);
  pantallaEmpujarTodo();
}

static void loopModo()
{
  if (cabeceraAtras()) { irAPantalla(s_volver); return; }

  if (toqueNuevo) { s_ax = toqueX; s_ay = toqueY; }
  if (!toqueActivo || !toquePropio || toqueY0 < AREA_Y) return;

  const int16_t d = s_modo->grados ? GRADOS : 1;
  bool cambio = false;
  while (toqueX - s_ax >= PASO_PX) { s_ax += PASO_PX; cambio = paso(s_modo->der, s_h, +d); }
  while (s_ax - toqueX >= PASO_PX) { s_ax -= PASO_PX; cambio = paso(s_modo->izq, s_h, -d); }
  while (s_ay - toqueY >= PASO_PX) { s_ay -= PASO_PX; cambio = paso(s_modo->arr, s_v, +d); }
  while (toqueY - s_ay >= PASO_PX) { s_ay += PASO_PX; cambio = paso(s_modo->aba, s_v, -d); }

  if (cambio) dibujarContador();
}

void screenSwRotarEntrar() { entrarModo(&ROTAR); }
void screenSwRotarLoop()   { loopModo(); }
void screenSwRotarSalir()  {}

void screenSwMoverEntrar() { entrarModo(&MOVER); }
void screenSwMoverLoop()   { loopModo(); }
void screenSwMoverSalir()  {}
