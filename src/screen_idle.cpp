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

// ---- Estado ---------------------------------------------------------------
static LGFX_Sprite s_spReloj(&lcd);
static bool      s_spOk    = false;
static RelojModo s_modo    = (RelojModo)RELOJ_POR_DEFECTO;

static int      s_ultSeg   = -1;
static int      s_ultMin   = -1;
static uint32_t s_ultNotif = 0xFFFFFFFF;

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
  g->drawString("toca para desbloquear", ox + CARD_W / 2, oy + CARD_H - 22);

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

// ---------------------------------------------------------------------------
//  Lista de notificaciones
// ---------------------------------------------------------------------------
static void dibujarLista()
{
  const Tema &tm_ = T();
  const int16_t y0 = PANEL_Y + CAB_H + 10;

  uint8_t total = notifCantidad();

  for (uint8_t i = 0; i < NOTIF_VISIBLES; i++) {
    int16_t y = y0 + i * (NOTIF_H + NOTIF_SEP);

    lcd.fillRoundRect(PANEL_X, y, PANEL_W, NOTIF_H, 14, tm_.superficie);

    if (i >= total) {                       // hueco vacio
      lcd.drawRoundRect(PANEL_X, y, PANEL_W, NOTIF_H, 14, tm_.superficie);
      continue;
    }

    const Notificacion &n = notifObtener(i);
    uint16_t barra = (n.tipo == NOTIF_ALERTA)  ? COL_ALERTA
                   : (n.tipo == NOTIF_MENSAJE) ? tm_.acento
                                               : tm_.acentoAlt;

    lcd.fillRoundRect(PANEL_X + 8, y + 10, 5, NOTIF_H - 20, 3, barra);

    char buf[NOTIF_TEXTO_LEN];
    lcd.setTextSize(1.0f);
    lcd.setTextDatum(top_left);

    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(tm_.texto);
    recortar(buf, sizeof(buf), n.titulo, 20);
    lcd.drawString(buf, PANEL_X + 22, y + 12);

    lcd.setTextColor(tm_.textoTenue);
    recortar(buf, sizeof(buf), n.texto, 28);
    lcd.drawString(buf, PANEL_X + 22, y + 38);

    lcd.setTextDatum(top_right);
    lcd.setTextColor(barra);
    lcd.drawString(n.hora, PANEL_X + PANEL_W - 12, y + 12);
  }

  // pie: cuantas quedan fuera de la vista
  const int16_t yPie = y0 + NOTIF_VISIBLES * (NOTIF_H + NOTIF_SEP);
  lcd.fillRect(PANEL_X, yPie, PANEL_W, ALTO - yPie, tm_.fondo);
  if (total > NOTIF_VISIBLES) {
    char pie[24];
    snprintf(pie, sizeof(pie), "+%u mas", total - NOTIF_VISIBLES);
    lcd.setFont(&fonts::Font0);
    lcd.setTextDatum(top_right);
    lcd.setTextColor(tm_.textoTenue, tm_.fondo);
    lcd.drawString(pie, PANEL_X + PANEL_W - 12, yPie + 2);
  }
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

  s_ultSeg = -1; s_ultMin = -1; s_ultNotif = 0xFFFFFFFF;

  lcd.setBrightness(brilloNormal < BRILLO_IDLE ? brilloNormal : BRILLO_IDLE);
  lcd.fillScreen(T().fondo);
}

void screenIdleSalir()
{
  if (s_spOk) { s_spReloj.deleteSprite(); s_spOk = false; }
  lcd.setBrightness(brilloNormal);
}

void screenIdleLoop()
{
  // ---- toque: dentro del reloj cambia de modo, fuera desbloquea ----
  if (toqueNuevo) {
    bool enReloj = (toqueX >= CARD_X && toqueX < CARD_X + CARD_W &&
                    toqueY >= CARD_Y && toqueY < CARD_Y + CARD_H);
    if (enReloj) {
      relojAlternarModo();
      Serial.printf("[idle] reloj %s\n",
                    s_modo == RELOJ_ANALOGICO ? "analogico" : "digital");
    } else {
      salirDeIdle();
      return;
    }
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
    dibujarLista();
    dibujarCabecera(t, valido);     // el chip del contador vive en la cabecera
  }

  if (t.tm_sec != s_ultSeg || cambioValidez) {
    s_ultSeg = t.tm_sec;
    dibujarReloj(t, valido);
  }
}
