#include "Pantallas/screen_idle.h"
#include "Drivers/display.h"
#include "Drivers/tiempo.h"
#include "Drivers/notificaciones.h"
#include "Estaticos/temas.h"
#include "Estaticos/reloj.h"
#include "config.h"

// ---- Geometria ------------------------------------------------------------
#define CARD_X   10
#define CARD_Y   12
#define CARD_W  180
#define CARD_H  296
#define CARD_R   22

#define PANEL_X 200
#define PANEL_Y  12
#define PANEL_W 270

#define CAB_H    46          // alto de la cabecera (fecha + chips)
#define NOTIF_H  72          // alto de cada tarjeta de notificacion
#define NOTIF_SEP 8
#define NOTIF_VISIBLES 3

static const uint16_t COL_ALERTA = rgb565(198, 70, 52);

#define LISTA_Y     (PANEL_Y + CAB_H + 10)
#define LISTA_H     (ALTO - LISTA_Y)
#define PASO_SCROLL ((NOTIF_H + NOTIF_SEP) / 2)

// ---- Estado ---------------------------------------------------------------
static LGFX_Sprite s_spReloj(&lcd);
static bool      s_spOk    = false;
static RelojModo s_modo    = (RelojModo)RELOJ_POR_DEFECTO;

static int      s_ultSeg   = -1;
static int      s_ultMin   = -1;
static uint32_t s_ultNotif = 0xFFFFFFFF;

static LGFX_Sprite s_spLista(&lcd);
static bool      s_spListaOk = false;
static int16_t   s_primera    = 0;
static int16_t   s_primeraIni = 0;

// ---------------------------------------------------------------------------
RelojModo relojModo() { return s_modo; }

void relojUsarModo(RelojModo m)
{
  s_modo = m;
  s_ultSeg = -1;          // fuerza redibujo
}

void relojAlternarModo()
{
  relojUsarModo(s_modo == RELOJ_ANALOGICO ? RELOJ_DIGITAL : RELOJ_ANALOGICO);
}

// Recorta un texto a lo ancho disponible (fuentes de ancho fijo aproximado)
static void recortar(char *destino, size_t cap, const char *origen, uint8_t maxCar)
{
  size_t n = strlen(origen);
  if (n <= maxCar) { strncpy(destino, origen, cap - 1); destino[cap - 1] = '\0'; return; }
  if (maxCar + 1 > cap) maxCar = cap - 2;
  memcpy(destino, origen, maxCar - 2);
  destino[maxCar - 2] = '.';
  destino[maxCar - 1] = '.';
  destino[maxCar]     = '\0';
}

// ---------------------------------------------------------------------------
//  Tarjeta del reloj (se redibuja cada segundo)
// ---------------------------------------------------------------------------
static void dibujarReloj(const struct tm &t, bool valido)
{
  const Tema &tm_ = T();
  lgfx::LGFXBase *g;
  int16_t ox, oy;

  if (s_spOk) { g = &s_spReloj; ox = 0; oy = 0; s_spReloj.fillSprite(tm_.fondo); }
  else        { g = &lcd;       ox = CARD_X; oy = CARD_Y; }

  g->fillRoundRect(ox, oy, CARD_W, CARD_H, CARD_R, tm_.superficie);
  g->drawRoundRect(ox, oy, CARD_W, CARD_H, CARD_R, tm_.borde);

  if (s_modo == RELOJ_ANALOGICO) {
    relojAnalogico(g, ox + CARD_W / 2, oy + 118, 82, t, valido);
  } else {
    relojDigital(g, ox, oy + 10, CARD_W, 200, t, valido);
  }

  // fecha al pie de la tarjeta
  char fecha[24];
  if (valido) snprintf(fecha, sizeof(fecha), "%s %02d %s",
                       DIA_CORTO[t.tm_wday % 7], t.tm_mday, MES_CORTO[t.tm_mon % 12]);
  else        strcpy(fecha, "-- -- ---");

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_center);
  g->setTextColor(tm_.acento);
  g->drawString(fecha, ox + CARD_W / 2, oy + CARD_H - 48);

  g->setFont(&fonts::Font0);
  g->setTextColor(tm_.textoTenue);
  g->drawString("desliza a la derecha", ox + CARD_W / 2, oy + CARD_H - 22);

  if (s_spOk) s_spReloj.pushSprite(CARD_X, CARD_Y);
}

// ---------------------------------------------------------------------------
//  Cabecera del panel derecho: dia, fecha y chips de estado
// ---------------------------------------------------------------------------
static void dibujarCabecera(const struct tm &t, bool valido)
{
  const Tema &tm_ = T();

  lcd.fillRect(PANEL_X, PANEL_Y, PANEL_W, CAB_H, tm_.fondo);
  lcd.setTextSize(1.0f);
  lcd.setTextDatum(top_left);

  char titulo[24];
  if (valido) snprintf(titulo, sizeof(titulo), "%s %d", DIA_LARGO[t.tm_wday % 7], t.tm_mday);
  else        strcpy(titulo, "Sin hora");

  lcd.setFont(&fonts::Font4);
  lcd.setTextColor(tm_.texto, tm_.fondo);
  lcd.drawString(titulo, PANEL_X, PANEL_Y + 2);

  if (valido) {
    char sub[24];
    snprintf(sub, sizeof(sub), "%s %d", MES_CORTO[t.tm_mon % 12], t.tm_year + 1900);
    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(tm_.textoTenue, tm_.fondo);
    lcd.drawString(sub, PANEL_X, PANEL_Y + 28);
  }

  // chips a la derecha: WiFi y contador de notificaciones
  const int16_t chipH = 20;
  int16_t cx = PANEL_X + PANEL_W;

  uint8_t n = notifCantidad();
  if (n) {
    char txt[8];
    snprintf(txt, sizeof(txt), "%u", n);
    int16_t w = 26;
    cx -= w;
    lcd.fillRoundRect(cx, PANEL_Y + 2, w, chipH, chipH / 2, tm_.acento);
    lcd.setFont(&fonts::Font2);
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(tm_.textoSobreAcento);
    lcd.drawString(txt, cx + w / 2, PANEL_Y + 2 + chipH / 2);
    cx -= 8;
  }

  const bool wifi = wifiConectado();
  int16_t w = 46;
  cx -= w;
  lcd.fillRoundRect(cx, PANEL_Y + 2, w, chipH, chipH / 2,
                    wifi ? tm_.acentoAlt : tm_.superficie);
  lcd.setFont(&fonts::Font2);
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(wifi ? tm_.textoSobreAcento : tm_.textoTenue);
  lcd.drawString("WiFi", cx + w / 2, PANEL_Y + 2 + chipH / 2);
}

static int16_t maxPrimera()
{
  const uint8_t n = notifCantidad();
  return n > NOTIF_VISIBLES ? n - NOTIF_VISIBLES : 0;
}

static bool enLista(int32_t x, int32_t y)
{
  return x >= PANEL_X && x < PANEL_X + PANEL_W && y >= LISTA_Y;
}

static bool enReloj(int32_t x, int32_t y)
{
  return x >= CARD_X && x < CARD_X + CARD_W && y >= CARD_Y && y < CARD_Y + CARD_H;
}

static void dibujarLista()
{
  const Tema &tm_ = T();
  lgfx::LGFXBase *g = s_spListaOk ? (lgfx::LGFXBase *)&s_spLista : (lgfx::LGFXBase *)&lcd;
  const int16_t ox = s_spListaOk ? 0 : PANEL_X;
  const int16_t oy = s_spListaOk ? 0 : LISTA_Y;
  const uint8_t total = notifCantidad();

  if (s_primera > maxPrimera()) s_primera = maxPrimera();

  g->fillRect(ox, oy, PANEL_W, LISTA_H, tm_.fondo);
  g->setTextSize(1.0f);

  for (uint8_t f = 0; f < NOTIF_VISIBLES; f++) {
    const int16_t y = oy + f * (NOTIF_H + NOTIF_SEP);
    const uint8_t i = s_primera + f;

    g->fillRoundRect(ox, y, PANEL_W, NOTIF_H, 14, tm_.superficie);
    if (i >= total) continue;

    const Notificacion &n = notifObtener(i);
    const uint16_t barra = (n.tipo == NOTIF_ALERTA)  ? COL_ALERTA
                         : (n.tipo == NOTIF_MENSAJE) ? tm_.acento
                                                     : tm_.acentoAlt;

    g->fillRoundRect(ox + 8, y + 10, 5, NOTIF_H - 20, 3, barra);

    char buf[NOTIF_TEXTO_LEN];
    g->setTextDatum(top_left);
    g->setFont(&fonts::Font2);
    g->setTextColor(tm_.texto);
    recortar(buf, sizeof(buf), n.titulo, 20);
    g->drawString(buf, ox + 22, y + 12);

    g->setTextColor(tm_.textoTenue);
    recortar(buf, sizeof(buf), n.texto, 28);
    g->drawString(buf, ox + 22, y + 38);

    g->setTextDatum(top_right);
    g->setTextColor(barra);
    g->drawString(n.hora, ox + PANEL_W - 12, y + 12);
  }

  if (total > NOTIF_VISIBLES) {
    char pie[24];
    snprintf(pie, sizeof(pie), "%u-%u de %u", s_primera + 1, s_primera + NOTIF_VISIBLES, total);
    g->setFont(&fonts::Font0);
    g->setTextDatum(top_right);
    g->setTextColor(tm_.textoTenue);
    g->drawString(pie, ox + PANEL_W - 12, oy + NOTIF_VISIBLES * (NOTIF_H + NOTIF_SEP) + 2);
  }

  if (s_spListaOk) s_spLista.pushSprite(PANEL_X, LISTA_Y);
}

// ---------------------------------------------------------------------------
void screenIdleEntrar()
{
  // El sprite del reloj es grande (180x296): primero PSRAM, luego RAM interna
  s_spReloj.setColorDepth(16);
  if (psramFound()) {
    s_spReloj.setPsram(true);
    s_spOk = s_spReloj.createSprite(CARD_W, CARD_H);
  }
  if (!s_spOk) {
    s_spReloj.setPsram(false);
    s_spOk = s_spReloj.createSprite(CARD_W, CARD_H);
  }

  s_spLista.setColorDepth(16);
  s_spLista.setPsram(psramFound());
  s_spListaOk = s_spLista.createSprite(PANEL_W, LISTA_H);
  s_primera = 0;

  s_ultSeg = -1; s_ultMin = -1; s_ultNotif = 0xFFFFFFFF;

  lcd.setBrightness(brilloNormal < BRILLO_IDLE ? brilloNormal : BRILLO_IDLE);
  lcd.fillScreen(T().fondo);
}

void screenIdleSalir()
{
  if (s_spOk) { s_spReloj.deleteSprite(); s_spOk = false; }
  if (s_spListaOk) { s_spLista.deleteSprite(); s_spListaOk = false; }
  lcd.setBrightness(brilloNormal);
}

void screenIdleLoop()
{
  if (toqueNuevo) s_primeraIni = s_primera;

  if (toqueActivo && toquePropio && enLista(toqueX0, toqueY0)) {
    const int32_t dy = toqueY - toqueY0;
    if (absI(dy) > absI(toqueX - toqueX0)) {
      const int16_t p = constrain(s_primeraIni - (int16_t)(dy / PASO_SCROLL), 0, maxPrimera());
      if (p != s_primera) { s_primera = p; dibujarLista(); }
    }
  }

  if (deslizoDerecha()) { salirDeIdle(); return; }

  if (fueToque() && enReloj(toqueX0, toqueY0)) {
    relojAlternarModo();
    Serial.printf("[idle] reloj %s\n", s_modo == RELOJ_ANALOGICO ? "analogico" : "digital");
  }

  struct tm t;
  bool valido = tiempoObtener(t);
  if (!valido) { memset(&t, 0, sizeof(t)); }

  static bool s_ultValido = false;
  bool cambioValidez = (valido != s_ultValido);
  s_ultValido = valido;

  if (t.tm_min != s_ultMin || cambioValidez) {
    s_ultMin = t.tm_min;
    dibujarCabecera(t, valido);
  }

  if (notifVersion() != s_ultNotif) {
    s_ultNotif = notifVersion();
    s_primera = 0;
    dibujarLista();
    dibujarCabecera(t, valido);     // el chip del contador vive en la cabecera
  }

  if (t.tm_sec != s_ultSeg || cambioValidez) {
    s_ultSeg = t.tm_sec;
    dibujarReloj(t, valido);
  }
}
