#include "Pantallas/screen_slider.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Drivers/audio.h"
#include "config.h"

#define ZONA_Y   (CAB_ALTO + 4)
#define ZONA_H   (ALTO - ZONA_Y)
#define PISTA_X0 50
#define PISTA_W  (ANCHO - 2 * PISTA_X0)
#define PISTA_Y  150
#define PERILLA  18

struct SliderCfg {
  const char *titulo;
  int16_t min, max, paso;
  int16_t (*leer)();
  void (*escribir)(int16_t);
  void (*texto)(int16_t, char *, size_t);
  void (*alSoltar)();
};

static void fmtBrillo(int16_t v, char *b, size_t n) { snprintf(b, n, "%d%%", v); }

static void fmtReposo(int16_t v, char *b, size_t n)
{
  if (v < 60)           snprintf(b, n, "%d s", v);
  else if (v % 60 == 0) snprintf(b, n, "%d min", v / 60);
  else                  snprintf(b, n, "%d min %d s", v / 60, v % 60);
}

static int16_t leerBrillo() { return (int16_t)((brilloNormal * 100 + 127) / 255); }

static void escribirBrillo(int16_t v)
{
  brilloNormal = (uint8_t)(v * 255 / 100);
  lcd.setBrightness(brilloNormal);
}

static int16_t leerReposo() { return (int16_t)(idleTimeoutMs / 1000UL); }
static void escribirReposo(int16_t v) { idleTimeoutMs = (uint32_t)v * 1000UL; }

static void fmtVolumen(int16_t v, char *b, size_t n)
{
  if (v == 0) snprintf(b, n, "Silencio");
  else        snprintf(b, n, "%d%%", v);
}

static int16_t leerVolumen() { return volumen; }
static void escribirVolumen(int16_t v) { volumen = (uint8_t)v; }
static void probarVolumen() { audioReproducir(SONIDO_VOLUMEN); }

static const SliderCfg CFG_BRILLO  = { "Brillo",  BRILLO_MIN_PCT, 100,          5, leerBrillo,  escribirBrillo,  fmtBrillo,  nullptr };
static const SliderCfg CFG_REPOSO  = { "Reposo",  REPOSO_MIN_S,   REPOSO_MAX_S, 5, leerReposo,  escribirReposo,  fmtReposo,  nullptr };
static const SliderCfg CFG_VOLUMEN = { "Volumen", 0,              100,          5, leerVolumen, escribirVolumen, fmtVolumen, probarVolumen };

static const SliderCfg *s_cfg = nullptr;
static int16_t s_valor = 0;

static void dibujar(bool empujar = true)
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  const int16_t oy = ZONA_Y;
  char txt[24];

  g->fillRect(0, oy, ANCHO, ZONA_H, t.fondo);

  s_cfg->texto(s_valor, txt, sizeof(txt));
  g->setTextDatum(middle_center);
  g->setFont(&fonts::Font4);
  g->setTextSize(2.0f);
  g->setTextColor(t.texto);
  g->drawString(txt, ANCHO / 2, oy + 60);
  g->setTextSize(1.0f);

  const int16_t py = oy + PISTA_Y;
  const int16_t kx = PISTA_X0 + (int32_t)(s_valor - s_cfg->min) * PISTA_W / (s_cfg->max - s_cfg->min);
  int16_t lleno = kx - PISTA_X0 + 7;
  if (lleno > PISTA_W) lleno = PISTA_W;
  g->fillRoundRect(PISTA_X0, py - 7, PISTA_W, 14, 7, t.superficieAlt);
  g->fillRoundRect(PISTA_X0, py - 7, lleno, 14, 7, t.acento);
  g->fillCircle(kx, py, PERILLA, t.primario);
  g->drawCircle(kx, py, PERILLA, t.acento);

  g->setFont(&fonts::Font2);
  g->setTextColor(t.textoTenue);
  s_cfg->texto(s_cfg->min, txt, sizeof(txt));
  g->setTextDatum(top_left);
  g->drawString(txt, PISTA_X0, py + PERILLA + 10);
  s_cfg->texto(s_cfg->max, txt, sizeof(txt));
  g->setTextDatum(top_right);
  g->drawString(txt, PISTA_X0 + PISTA_W, py + PERILLA + 10);

  if (empujar) pantallaEmpujar(0, ZONA_Y, ANCHO, ZONA_H);
}

static int16_t valorEn(int32_t x)
{
  if (x < PISTA_X0) x = PISTA_X0;
  if (x > PISTA_X0 + PISTA_W) x = PISTA_X0 + PISTA_W;
  const int32_t rango = s_cfg->max - s_cfg->min;
  int32_t v = (x - PISTA_X0) * rango / PISTA_W;
  v = ((v + s_cfg->paso / 2) / s_cfg->paso) * s_cfg->paso + s_cfg->min;
  return (int16_t)(v > s_cfg->max ? s_cfg->max : v);
}

static void entrar(const SliderCfg &c)
{
  s_cfg   = &c;
  s_valor = constrain(c.leer(), c.min, c.max);

  pantalla()->fillScreen(T().fondo);
  cabeceraDibujar(c.titulo, "< Ajustes");
  dibujar(false);
  pantallaEmpujarTodo();
}

static void loopSlider()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_AJUSTES); return; }

  if (toqueActivo && toquePropio && toqueY0 >= ZONA_Y) {
    const int16_t v = valorEn(toqueX);
    if (v != s_valor) {
      s_valor = v;
      s_cfg->escribir(v);
      dibujar();
    }
  }

  if (toqueSuelto && toqueY0 >= ZONA_Y && s_cfg->alSoltar) s_cfg->alSoltar();
}

static void salirSlider() {}

void screenBrilloEntrar() { entrar(CFG_BRILLO); }
void screenBrilloLoop()   { loopSlider(); }
void screenBrilloSalir()  { salirSlider(); }

void screenReposoEntrar() { entrar(CFG_REPOSO); }
void screenReposoLoop()   { loopSlider(); }
void screenReposoSalir()  { salirSlider(); }

void screenVolumenEntrar() { entrar(CFG_VOLUMEN); }
void screenVolumenLoop()   { loopSlider(); }
void screenVolumenSalir()  { salirSlider(); }

const char *textoBrillo()
{
  static char b[12];
  fmtBrillo(leerBrillo(), b, sizeof(b));
  return b;
}

const char *textoVolumen()
{
  static char b[12];
  fmtVolumen(volumen, b, sizeof(b));
  return b;
}

const char *textoReposo()
{
  static char b[16];
  fmtReposo(leerReposo(), b, sizeof(b));
  return b;
}
