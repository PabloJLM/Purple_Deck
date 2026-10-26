#include "Estaticos/rejilla.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Drivers/acciones.h"

#define MARCA_MS         350    // cuanto queda marcado un boton de accion
#define ICONO_LADO        64    // todos los iconos son de 64x64
#define SLOTS_POR_PAG      7
#define CELDAS             9    // rejilla 3x3

enum CeldaTipo : uint8_t { CELDA_VACIA = 0, CELDA_ITEM, CELDA_ATRAS, CELDA_SIG, CELDA_ANT };

// Celdas (0..8, de izquierda a derecha y de arriba a abajo) que llevan items:
// todas menos la 6 (Atras) y la 8 (Siguiente).
static const uint8_t SLOT_CELDA[SLOTS_POR_PAG] = { 0, 1, 2, 3, 4, 5, 7 };
#define CELDA_ATRAS_POS 6
#define CELDA_SIG_POS   8

static const RejillaCfg *s_cfg = nullptr;
static uint8_t  s_n       = 0;
static uint8_t  s_pagina  = 0;
static uint8_t  s_paginas = 1;

static int16_t  s_x[CELDAS];
static int16_t  s_y[CELDAS];
static uint8_t  s_tipo[CELDAS];
static int8_t   s_item[CELDAS];          // indice de item en la celda, o -1
static uint8_t  s_ncel = 0;
static bool     s_icono[REJILLA_MAX_ITEMS];
static int16_t  BW, BH;

static LGFX_Sprite s_lienzo(&lcd);       // un boton a la vez, sin parpadeo
static bool     s_usarSprite = false;

static int8_t   s_activo = -1;           // celda marcada
static uint32_t s_tPres  = 0;

// ---------------------------------------------------------------------------
//  Layout: la rejilla llena toda la pantalla
// ---------------------------------------------------------------------------
static void calcular()
{
  const uint8_t maxN = s_cfg->paginada ? REJILLA_MAX_ITEMS : CELDAS;
  s_n = (s_cfg->n > maxN) ? maxN : s_cfg->n;

  const uint8_t cols = s_cfg->paginada ? 3 : (s_cfg->cols ? s_cfg->cols : 3);
  uint8_t rows = s_cfg->paginada ? 3 : (uint8_t)((s_n + cols - 1) / cols);
  if (rows == 0) rows = 1;

  const int16_t cw = ANCHO / cols;
  const int16_t ch = ALTO  / rows;

  BW = cw - GAP;
  BH = ch - GAP;

  const int16_t offX = (ANCHO - cw * cols) / 2;
  const int16_t offY = (ALTO  - ch * rows) / 2;

  s_ncel = s_cfg->paginada ? CELDAS : s_n;
  for (uint8_t c = 0; c < s_ncel; c++) {
    s_x[c] = offX + (c % cols) * cw + GAP / 2;
    s_y[c] = offY + (c / cols) * ch + GAP / 2;
  }

  for (uint8_t i = 0; i < s_n; i++) {
    const char *ic = s_cfg->items[i].icono;
    s_icono[i] = ic && fsDisponible() && LittleFS.exists(ic);
  }
}

// Decide que lleva cada celda segun la pagina actual
static void asignarCeldas()
{
  for (uint8_t c = 0; c < CELDAS; c++) { s_tipo[c] = CELDA_VACIA; s_item[c] = -1; }

  if (!s_cfg->paginada) {
    for (uint8_t i = 0; i < s_n; i++) { s_tipo[i] = CELDA_ITEM; s_item[i] = i; }
    return;
  }

  s_paginas = (uint8_t)((s_n + SLOTS_POR_PAG - 1) / SLOTS_POR_PAG);
  if (s_paginas == 0) s_paginas = 1;
  if (s_pagina >= s_paginas) s_pagina = 0;

  if (!s_cfg->anterior)      s_tipo[CELDA_ATRAS_POS] = CELDA_ATRAS;
  else if (s_paginas > 1)    s_tipo[CELDA_ATRAS_POS] = CELDA_ANT;
  if (s_paginas > 1) s_tipo[CELDA_SIG_POS] = CELDA_SIG;

  for (uint8_t k = 0; k < SLOTS_POR_PAG; k++) {
    const uint8_t idx = s_pagina * SLOTS_POR_PAG + k;
    if (idx >= s_n) break;
    s_tipo[SLOT_CELDA[k]] = CELDA_ITEM;
    s_item[SLOT_CELDA[k]] = idx;
  }
}

// Separa la etiqueta en una o dos lineas
static uint8_t partirEtiqueta(const char *e, char *l1, char *l2, size_t cap)
{
  const char *nl = strchr(e, '\n');
  if (!nl) {
    strncpy(l1, e, cap - 1); l1[cap - 1] = '\0';
    l2[0] = '\0';
    return 1;
  }
  size_t n1 = (size_t)(nl - e);
  if (n1 >= cap) n1 = cap - 1;
  memcpy(l1, e, n1); l1[n1] = '\0';
  strncpy(l2, nl + 1, cap - 1); l2[cap - 1] = '\0';
  return 2;
}

static void usarFuente(lgfx::LGFXBase *g, bool chico)
{
  if (chico) g->setFont(&fonts::Font2);
  else       g->setFont(&fonts::Font4);
}

// ---------------------------------------------------------------------------
//  Dibujo de un item sobre 'g' (pantalla o sprite) con origen (ox, oy)
// ---------------------------------------------------------------------------
static void pintarItem(lgfx::LGFXBase *g, uint8_t idx, bool presionado, int16_t ox, int16_t oy)
{
  const Tema &t  = T();
  const Item &it = s_cfg->items[idx];

  const uint16_t fondo = presionado ? t.superficieAlt : t.primario;
  const uint16_t texto = presionado ? t.acento        : t.texto;

  g->setTextSize(1.0f);
  g->fillRoundRect(ox, oy, BW, BH, RAD, fondo);
  g->drawRoundRect(ox, oy, BW, BH, RAD, presionado ? t.acento : t.borde);

  const bool conIcono = s_icono[idx];
  // Celda baja (3x3) con icono: etiqueta de una sola linea y chica
  const bool compacto = conIcono && (BH < 120);

  char l1[48], l2[24];
  uint8_t lineas = partirEtiqueta(it.etiqueta, l1, l2, 24);
  if (compacto && lineas == 2) {
    char tmp[48];
    snprintf(tmp, sizeof(tmp), "%s %s", l1, l2);
    strcpy(l1, tmp);
    lineas = 1;
  }

  // Font4 si todas las lineas caben, si no Font2
  const int16_t maxW = BW - 12;
  bool chico = compacto;
  if (!chico) {
    g->setFont(&fonts::Font4);
    chico = (g->textWidth(l1) > maxW) || (lineas == 2 && g->textWidth(l2) > maxW);
  }
  usarFuente(g, chico);
  const int16_t altoLinea = g->fontHeight();

  const char *sub = it.subDin ? it.subDin() : it.sub;
  const bool haySub = (sub && sub[0]) && !compacto;
  const int16_t altoSub = 16;                       // Font2
  const int16_t sepIcono = compacto ? 4 : 6;

  // Altura total del contenido para centrarlo verticalmente
  int16_t total = lineas * altoLinea;
  if (conIcono) total += ICONO_LADO + sepIcono;
  if (haySub)   total += altoSub + 4;

  int16_t y = oy + (BH - total) / 2;
  const int16_t cx = ox + BW / 2;

  g->setTextColor(texto);
  g->setTextDatum(middle_center);

  if (conIcono) {
    // (x, y) es la esquina de la caja ICONO_LADO x ICONO_LADO; el datum centra dentro de ella
    g->drawPngFile(LittleFS, it.icono, cx - ICONO_LADO / 2, y, ICONO_LADO, ICONO_LADO,
                   0, 0, 0.0f, 0.0f, datum_t::middle_center);
    y += ICONO_LADO + sepIcono;
  }

  usarFuente(g, chico);
  g->setTextColor(texto);
  g->drawString(l1, cx, y + altoLinea / 2);
  y += altoLinea;
  if (lineas == 2) {
    g->drawString(l2, cx, y + altoLinea / 2);
    y += altoLinea;
  }

  if (haySub) {
    y += 4;
    g->setFont(&fonts::Font2);
    g->drawString(sub, cx, y + altoSub / 2);
  }
}

// Botones de navegacion: Atras y Siguiente
static void pintarNav(lgfx::LGFXBase *g, uint8_t tipo, int16_t ox, int16_t oy)
{
  const Tema &t = T();

  g->setTextSize(1.0f);
  g->fillRoundRect(ox, oy, BW, BH, RAD, t.superficie);
  g->drawRoundRect(ox, oy, BW, BH, RAD, t.borde);

  g->setTextDatum(middle_center);
  g->setTextColor(t.acento);

  const int16_t cx = ox + BW / 2;
  const int16_t cy = oy + BH / 2;

  g->setFont(&fonts::Font4);
  if (tipo == CELDA_ATRAS) {
    g->drawString("< Menu", cx, cy);
  } else {
    char pg[12];
    snprintf(pg, sizeof(pg), "%u/%u", (unsigned)(s_pagina + 1), (unsigned)s_paginas);
    g->drawString(tipo == CELDA_ANT ? "< Ant" : "Sig >", cx, cy - 12);
    g->setFont(&fonts::Font2);
    g->setTextColor(t.texto);
    g->drawString(pg, cx, cy + 16);
  }
}

static void pintarCelda(lgfx::LGFXBase *g, uint8_t c, bool presionado, int16_t ox, int16_t oy)
{
  if (s_tipo[c] == CELDA_ITEM) pintarItem(g, (uint8_t)s_item[c], presionado, ox, oy);
  else                         pintarNav(g, s_tipo[c], ox, oy);
}

static void dibujar(uint8_t c, bool presionado)
{
  if (s_tipo[c] == CELDA_VACIA) {            // hueco libre: solo fondo
    lcd.fillRect(s_x[c], s_y[c], BW, BH, T().fondo);
    return;
  }
  if (s_usarSprite) {
    s_lienzo.fillSprite(T().fondo);
    pintarCelda(&s_lienzo, c, presionado, 0, 0);
    s_lienzo.pushSprite(s_x[c], s_y[c]);
  } else {
    pintarCelda(&lcd, c, presionado, s_x[c], s_y[c]);
  }
}

static int8_t celdaEn(int32_t x, int32_t y)
{
  for (uint8_t c = 0; c < s_ncel; c++)
    if (x >= s_x[c] && x < s_x[c] + BW && y >= s_y[c] && y < s_y[c] + BH) return c;
  return -1;
}

// ---------------------------------------------------------------------------
void rejillaRedibujar(bool limpiar)
{
  if (!s_cfg) return;

  if (pantallaDoble()) {
    lgfx::LGFXBase *g = pantalla();
    g->fillRect(0, 0, ANCHO, ALTO, T().fondo);
    for (uint8_t c = 0; c < s_ncel; c++)
      if (s_tipo[c] != CELDA_VACIA) pintarCelda(g, c, false, s_x[c], s_y[c]);
    pantallaEmpujarTodo();
    return;
  }

  if (limpiar) lcd.fillScreen(T().fondo);
  for (uint8_t c = 0; c < s_ncel; c++) dibujar(c, false);
}

void rejillaEntrar(const RejillaCfg &c)
{
  s_cfg    = &c;
  s_pagina = 0;
  calcular();
  asignarCeldas();

  if (s_usarSprite) { s_lienzo.deleteSprite(); s_usarSprite = false; }
  s_lienzo.setColorDepth(16);
  s_usarSprite = s_lienzo.createSprite(BW, BH);
  if (!s_usarSprite && psramFound()) {
    s_lienzo.setPsram(true);
    s_usarSprite = s_lienzo.createSprite(BW, BH);
  }

  s_activo = -1;
  rejillaRedibujar(true);
}

void rejillaSalir()
{
  if (s_usarSprite) { s_lienzo.deleteSprite(); s_usarSprite = false; }
}

void rejillaLoop()
{
  if (!s_cfg) return;

  if (toqueNuevo) {
    const int8_t c = celdaEn(toqueX, toqueY);

    if (c >= 0) {
      switch (s_tipo[c]) {

        case CELDA_ATRAS:
          irAPantalla(s_cfg->atras);
          return;

        case CELDA_ANT:
        case CELDA_SIG:
          s_pagina = (uint8_t)((s_pagina + (s_tipo[c] == CELDA_SIG ? 1 : s_paginas - 1)) % s_paginas);
          asignarCeldas();
          s_activo = -1;
          rejillaRedibujar(false);        // misma rejilla: basta repintar las celdas
          break;

        case CELDA_ITEM: {
          const Item &it = s_cfg->items[s_item[c]];

          if (it.tipo == ITEM_PANTALLA) {
            irAPantalla((Screen)it.valor);
            return;
          }

          // Accion: se ejecuta y el boton queda marcado un instante
          if (s_activo >= 0 && s_activo != c) dibujar(s_activo, false);
          s_activo = c;
          s_tPres  = millis();

          const bool redibujarTodo = accionEjecutar((Accion)it.valor);
          if (redibujarTodo) {
            rejillaRedibujar(true);
            s_activo = -1;
          } else {
            dibujar(c, true);             // despues de ejecutar: el texto dinamico ya cambio
          }
          break;
        }

        default:
          break;
      }
    }
  }

  if (s_activo >= 0 && (millis() - s_tPres > MARCA_MS)) {
    dibujar(s_activo, false);
    s_activo = -1;
  }
}
