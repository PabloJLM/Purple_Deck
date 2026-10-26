#include "Pantallas/screen_bluetooth.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/ui.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Drivers/teclado.h"
#include "config.h"

#define TARJETA_X  GAP
#define TARJETA_W  (ANCHO - 2 * GAP)
#define TARJETA_H  56
#define TEXTO_MAX  (TARJETA_W - 32)

#define BOTON_H 48
#define BOTON_Y (CAB_ALTO + 6 + 3 * (TARJETA_H + 6) + 20)

static bool s_conectadoPrev = false;
static uint32_t s_mensajeHasta = 0;

static void valor(const char *texto, int16_t y)
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  char b[72];
  strncpy(b, texto, sizeof(b) - 1);
  b[sizeof(b) - 1] = '\0';

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font4);
  if (g->textWidth(b) > TEXTO_MAX) g->setFont(&fonts::Font2);

  size_t n = strlen(b);
  bool cortado = false;
  while (n > 3 && g->textWidth(b) > TEXTO_MAX) {
    b[--n] = '\0';
    cortado = true;
  }
  if (cortado) { b[n - 1] = b[n - 2] = b[n - 3] = '.'; }

  g->setTextDatum(middle_left);
  g->setTextColor(t.texto);
  g->drawString(b, TARJETA_X + 16, y);
}

static void tarjeta(const char *etiqueta, const char *texto, int16_t y)
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  g->fillRoundRect(TARJETA_X, y, TARJETA_W, TARJETA_H, RAD, t.superficie);
  g->drawRoundRect(TARJETA_X, y, TARJETA_W, TARJETA_H, RAD, t.borde);

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);
  g->setTextDatum(top_left);
  g->setTextColor(t.acento);
  g->drawString(etiqueta, TARJETA_X + 16, y + 8);

  valor(texto, y + TARJETA_H - 20);
}

static void dibujar()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  const bool conectado = tecladoConectado();
  s_conectadoPrev = conectado;

  g->fillRect(0, CAB_ALTO, ANCHO, ALTO - CAB_ALTO, t.fondo);

  const int16_t y0 = CAB_ALTO + 6;

  char estado[48];
  if (conectado) {
    const char *mac = tecladoDireccion();
    snprintf(estado, sizeof(estado), "Conectado a %s", mac ? mac : "?");
  } else {
    snprintf(estado, sizeof(estado), "Desconectado - anunciandose");
  }
  tarjeta("Estado", estado, y0);
  tarjeta("Nombre Bluetooth", BLE_NOMBRE, y0 + (TARJETA_H + 6));
  tarjeta("Emparejamientos guardados", tecladoEmparejado() ? "Si" : "Ninguno", y0 + 2 * (TARJETA_H + 6));

  const char *msg = (millis() < s_mensajeHasta)
    ? "Listo. Busca \"Purple Deck\" en tu PC para emparejar."
    : "Usa esto si quieres conectar el deck a otra PC.";
  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_center);
  g->setTextColor(t.textoTenue);
  g->drawString(msg, ANCHO / 2, CAB_ALTO + 6 + 3 * (TARJETA_H + 6) + 6);

  botonDibujar(g, TARJETA_X, BOTON_Y, TARJETA_W, BOTON_H, "Reemparejar",
               t.acento, t.textoSobreAcento, &fonts::Font4);

  pantallaEmpujar(0, CAB_ALTO, ANCHO, ALTO - CAB_ALTO);
}

void screenBluetoothEntrar()
{
  s_mensajeHasta = 0;
  pantalla()->fillScreen(T().fondo);
  cabeceraDibujar("Bluetooth", "< Ajustes");
  dibujar();
  pantallaEmpujarTodo();
}

void screenBluetoothLoop()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_AJUSTES); return; }

  if (toqueEn(TARJETA_X, BOTON_Y, TARJETA_W, BOTON_H)) {
    tecladoReemparejar();
    s_mensajeHasta = millis() + 5000UL;
    dibujar();
    return;
  }

  if (tecladoConectado() != s_conectadoPrev) dibujar();
}

void screenBluetoothSalir() {}
