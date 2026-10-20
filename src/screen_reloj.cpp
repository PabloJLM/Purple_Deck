#include "Pantallas/screen_reloj.h"
#include "Pantallas/screen_idle.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Estaticos/reloj.h"
#include "Estaticos/ui.h"
#include "Drivers/display.h"
#include "Drivers/tiempo.h"
#include "Drivers/temporizadores.h"
#include "config.h"

#define TAB_Y  (CAB_ALTO + 2)
#define TAB_H  40
#define TABS   4
#define TAB_W  (ANCHO / TABS)
#define CY     (TAB_Y + TAB_H + 4)
#define CH     (ALTO - CY)

enum Tab : uint8_t { TAB_HORA = 0, TAB_ALARMAS, TAB_CRONO, TAB_TEMP };
static const char *TAB_NOMBRE[TABS] = { "Hora", "Alarmas", "Crono", "Temp" };

#define FILA_ALTO  34
#define FILA_PASO  38
#define PILDORA_W  84
#define EDIT_Y     (4 * FILA_PASO + 6)
#define EDIT_H     50
#define EDIT_W     108
#define EDIT_GAP   8

#define CR_Y     86
#define CR_H     56

#define MUESTRAS 100
#define MUESTRA_MS 2000UL

static LGFX_Sprite s_sp(&lcd);
static bool     s_spOk = false;
static Tab      s_tab = TAB_HORA;
static bool     s_sucio = true;
static int      s_segPrev = -1;
static uint32_t s_tRefresco = 0;
static uint8_t  s_sel = 0;
static bool     s_alarmaSucia = false;
static uint32_t s_repetir = 0;

static float    s_muestras[MUESTRAS];
static uint8_t  s_nMuestras = 0;
static uint32_t s_tMuestra = 0;

static lgfx::LGFXBase *destino() { return s_spOk ? (lgfx::LGFXBase *)&s_sp : (lgfx::LGFXBase *)&lcd; }
static int16_t origenY() { return s_spOk ? 0 : CY; }

static void dibujarTabs()
{
  const Tema &t = T();
  for (uint8_t i = 0; i < TABS; i++) {
    const bool act = (i == s_tab);
    botonDibujar(&lcd, i * TAB_W + 3, TAB_Y, TAB_W - 6, TAB_H, TAB_NOMBRE[i],
                 act ? t.acento : t.superficie, act ? t.textoSobreAcento : t.texto, &fonts::Font4);
  }
}

static void dibujarHora(lgfx::LGFXBase *g, int16_t oy)
{
  const Tema &t = T();
  struct tm tm_;
  const bool valido = tiempoObtener(tm_);

  if (relojModo() == RELOJ_ANALOGICO) relojAnalogico(g, ANCHO / 2, oy + 94, 86, tm_, valido);
  else                                relojDigital(g, 0, oy, ANCHO, 190, tm_, valido);

  char b[32];
  if (valido) snprintf(b, sizeof(b), "%s %02d %s %04d", DIA_CORTO[tm_.tm_wday], tm_.tm_mday, MES_CORTO[tm_.tm_mon], tm_.tm_year + 1900);
  else        snprintf(b, sizeof(b), "Sin hora (WiFi/NTP)");
  g->setTextSize(1.0f);
  g->setFont(&fonts::Font4);
  g->setTextDatum(middle_center);
  g->setTextColor(t.textoTenue);
  g->drawString(b, ANCHO / 2, oy + 203);
}

static void dibujarAlarmas(lgfx::LGFXBase *g, int16_t oy)
{
  const Tema &t = T();
  char b[8];

  for (uint8_t i = 0; i < NUM_ALARMAS; i++) {
    const Alarma &a = alarma(i);
    const int16_t y = oy + i * FILA_PASO;
    g->fillRoundRect(8, y, ANCHO - 16, FILA_ALTO, RAD, i == s_sel ? t.superficieAlt : t.superficie);
    g->drawRoundRect(8, y, ANCHO - 16, FILA_ALTO, RAD, i == s_sel ? t.acento : t.borde);

    snprintf(b, sizeof(b), "%02u:%02u", a.hora, a.minuto);
    g->setTextSize(1.0f);
    g->setFont(&fonts::Font4);
    g->setTextDatum(middle_left);
    g->setTextColor(a.activa ? t.texto : t.textoTenue);
    g->drawString(b, 24, y + FILA_ALTO / 2);

    botonDibujar(g, ANCHO - 8 - PILDORA_W - 4, y + 3, PILDORA_W, FILA_ALTO - 6, a.activa ? "ON" : "OFF",
                 a.activa ? t.acento : t.superficie, a.activa ? t.textoSobreAcento : t.textoTenue, &fonts::Font2);
  }

  static const char *ET[4] = { "Hora -", "Hora +", "Min -", "Min +" };
  for (uint8_t k = 0; k < 4; k++)
    botonDibujar(g, 8 + k * (EDIT_W + EDIT_GAP), oy + EDIT_Y, EDIT_W, EDIT_H, ET[k], t.superficie, t.texto, &fonts::Font4);
}

static void formatoCrono(uint32_t ms, char *b, size_t n)
{
  snprintf(b, n, "%02lu:%02lu.%02lu", (unsigned long)(ms / 60000UL % 100), (unsigned long)(ms / 1000UL % 60), (unsigned long)(ms / 10UL % 100));
}

static void dibujarCrono(lgfx::LGFXBase *g, int16_t oy)
{
  const Tema &t = T();
  char b[24];

  formatoCrono(cronoMs(), b, sizeof(b));
  g->setTextSize(1.0f);
  g->setFont(&fonts::Font7);
  g->setTextDatum(middle_center);
  g->setTextColor(t.texto);
  g->drawString(b, ANCHO / 2, oy + 40);

  botonDibujar(g, 8, oy + CR_Y, 228, CR_H, cronoCorriendo() ? "Pausa" : "Iniciar", t.acento, t.textoSobreAcento, &fonts::Font4);
  botonDibujar(g, 244, oy + CR_Y, 108, CR_H, "Vuelta", t.superficie, cronoCorriendo() ? t.texto : t.textoTenue, &fonts::Font4);
  botonDibujar(g, 360, oy + CR_Y, 112, CR_H, "Reiniciar", t.superficie, t.texto, &fonts::Font2);

  const uint8_t n = cronoVueltas() < 3 ? cronoVueltas() : 3;
  g->setFont(&fonts::Font4);
  g->setTextColor(t.textoTenue);
  for (uint8_t i = 0; i < n; i++) {
    char f[16];
    formatoCrono(cronoVueltaMs(i), f, sizeof(f));
    snprintf(b, sizeof(b), "#%u  %s", cronoVueltas() - i, f);
    g->drawString(b, ANCHO / 2, oy + 160 + i * 24);
  }
}

static void dibujarTemp(lgfx::LGFXBase *g, int16_t oy)
{
  const Tema &t = T();
  char b[16];
  const float v = s_nMuestras ? s_muestras[s_nMuestras - 1] : tempChip();

  snprintf(b, sizeof(b), "%.1f", v);
  g->setTextSize(1.0f);
  g->setFont(&fonts::Font7);
  g->setTextDatum(middle_center);
  g->setTextColor(t.texto);
  g->drawString(b, ANCHO / 2 - 20, oy + 38);
  const int16_t w = g->textWidth(b);
  g->setFont(&fonts::Font4);
  g->setTextColor(t.acento);
  g->setTextDatum(middle_left);
  g->drawString("C", ANCHO / 2 - 20 + w / 2 + 10, oy + 46);

  const int16_t gx = 20, gy = oy + 80, gw = ANCHO - 40, gh = 88;
  g->fillRoundRect(gx - 6, gy - 6, gw + 12, gh + 12, RAD, t.superficie);
  g->drawRoundRect(gx - 6, gy - 6, gw + 12, gh + 12, RAD, t.borde);
  if (s_nMuestras > 1) {
    float lo = s_muestras[0], hi = s_muestras[0];
    for (uint8_t i = 1; i < s_nMuestras; i++) { lo = min(lo, s_muestras[i]); hi = max(hi, s_muestras[i]); }
    if (hi - lo < 2.0f) { const float m = (hi + lo) / 2; lo = m - 1; hi = m + 1; }
    int16_t px = 0, py = 0;
    for (uint8_t i = 0; i < s_nMuestras; i++) {
      const int16_t x = gx + (int32_t)i * gw / (MUESTRAS - 1);
      const int16_t y = gy + gh - (int16_t)((s_muestras[i] - lo) / (hi - lo) * gh);
      if (i) g->drawLine(px, py, x, y, t.acento);
      px = x; py = y;
    }
    g->setFont(&fonts::Font2);
    g->setTextColor(t.textoTenue);
    g->setTextDatum(top_left);
    snprintf(b, sizeof(b), "%.0f", hi);
    g->drawString(b, gx + 2, gy);
    g->setTextDatum(bottom_left);
    snprintf(b, sizeof(b), "%.0f", lo);
    g->drawString(b, gx + 2, gy + gh);
  }

  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_center);
  g->setTextColor(t.textoTenue);
  g->drawString("Sensor interno del chip: marca mas que el ambiente", ANCHO / 2, oy + 196);
}

static void dibujarContenido()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = destino();
  const int16_t oy = origenY();

  g->fillRect(0, oy, ANCHO, CH, t.fondo);
  switch (s_tab) {
    case TAB_HORA:    dibujarHora(g, oy);    break;
    case TAB_ALARMAS: dibujarAlarmas(g, oy); break;
    case TAB_CRONO:   dibujarCrono(g, oy);   break;
    case TAB_TEMP:    dibujarTemp(g, oy);    break;
  }
  if (s_spOk) s_sp.pushSprite(0, CY);
}

static void guardarSiHaceFalta()
{
  if (s_alarmaSucia) { alarmasGuardar(); s_alarmaSucia = false; }
}

static bool repite(int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (!(toqueActivo && toquePropio && toqueX >= x && toqueX < x + w && toqueY >= y && toqueY < y + h)) return false;
  const uint32_t ahora = millis();
  if (toqueNuevo) { s_repetir = ahora + 400; return true; }
  if ((int32_t)(ahora - s_repetir) >= 0) { s_repetir = ahora + 120; return true; }
  return false;
}

static void tomarMuestra()
{
  if (s_nMuestras == MUESTRAS) {
    for (uint8_t i = 1; i < MUESTRAS; i++) s_muestras[i - 1] = s_muestras[i];
    s_nMuestras--;
  }
  s_muestras[s_nMuestras++] = tempChip();
}

static void entradaAlarmas()
{
  Alarma &a = alarma(s_sel);
  const int16_t ey = CY + EDIT_Y;

  for (uint8_t i = 0; i < NUM_ALARMAS; i++) {
    const int16_t y = CY + i * FILA_PASO;
    if (toqueEn(ANCHO - 8 - PILDORA_W - 4, y, PILDORA_W + 4, FILA_ALTO)) {
      alarma(i).activa = !alarma(i).activa;
      s_sel = i;
      s_alarmaSucia = true; s_sucio = true;
      return;
    }
    if (toqueEn(8, y, ANCHO - 16, FILA_ALTO)) { s_sel = i; s_sucio = true; return; }
  }

  const int16_t x0 = 8;
  if (repite(x0, ey, EDIT_W, EDIT_H))                            { a.hora   = (a.hora + 23) % 24;   s_alarmaSucia = s_sucio = true; }
  else if (repite(x0 + EDIT_W + EDIT_GAP, ey, EDIT_W, EDIT_H))   { a.hora   = (a.hora + 1) % 24;    s_alarmaSucia = s_sucio = true; }
  else if (repite(x0 + 2 * (EDIT_W + EDIT_GAP), ey, EDIT_W, EDIT_H)) { a.minuto = (a.minuto + 59) % 60; s_alarmaSucia = s_sucio = true; }
  else if (repite(x0 + 3 * (EDIT_W + EDIT_GAP), ey, EDIT_W, EDIT_H)) { a.minuto = (a.minuto + 1) % 60;  s_alarmaSucia = s_sucio = true; }

  if (!toqueActivo) guardarSiHaceFalta();
}

static void entradaCrono()
{
  if (toqueEn(8, CY + CR_Y, 228, CR_H))        { cronoIniciarPausar(); s_sucio = true; }
  else if (toqueEn(244, CY + CR_Y, 108, CR_H)) { cronoVuelta();        s_sucio = true; }
  else if (toqueEn(360, CY + CR_Y, 112, CR_H)) { cronoReiniciar();     s_sucio = true; }
}

void screenRelojEntrar()
{
  s_sp.setColorDepth(16);
  s_sp.setPsram(psramFound());
  s_spOk = s_sp.createSprite(ANCHO, CH);

  s_sucio = true;
  s_segPrev = -1;
  s_tRefresco = 0;
  s_tMuestra = 0;

  lcd.fillScreen(T().fondo);
  cabeceraDibujar("Reloj", "< Menu");
  dibujarTabs();
}

void screenRelojLoop()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_MENU); return; }

  for (uint8_t i = 0; i < TABS; i++) {
    if (i != s_tab && toqueEn(i * TAB_W, TAB_Y, TAB_W, TAB_H)) {
      guardarSiHaceFalta();
      s_tab = (Tab)i;
      dibujarTabs();
      s_sucio = true;
      s_segPrev = -1;
    }
  }

  const uint32_t ahora = millis();

  if (s_tMuestra == 0 || ahora - s_tMuestra >= MUESTRA_MS) {
    s_tMuestra = ahora ? ahora : 1;
    tomarMuestra();
    if (s_tab == TAB_TEMP) s_sucio = true;
  }

  switch (s_tab) {
    case TAB_ALARMAS: entradaAlarmas(); break;
    case TAB_CRONO:
      entradaCrono();
      if (cronoCorriendo() && ahora - s_tRefresco >= 50) s_sucio = true;
      break;
    case TAB_HORA: {
      struct tm t;
      const int seg = tiempoObtener(t) ? t.tm_sec : -2;
      if (seg != s_segPrev) { s_segPrev = seg; s_sucio = true; }
      break;
    }
    default: break;
  }

  if (s_sucio) {
    s_sucio = false;
    s_tRefresco = ahora;
    dibujarContenido();
  }
}

void screenRelojSalir()
{
  guardarSiHaceFalta();
  if (s_spOk) { s_sp.deleteSprite(); s_spOk = false; }
}
