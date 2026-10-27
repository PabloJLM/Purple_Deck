#include "Pantallas/screen_credenciales.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "config.h"
#include "Drivers/red.h"

#define TARJETA_X  GAP
#define TARJETA_W  (ANCHO - 2 * GAP)
#define TARJETA_H  84
#define TEXTO_MAX  (TARJETA_W - 32)

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
  g->drawString(etiqueta, TARJETA_X + 16, y + 10);

  valor(texto, y + 54);
}

void screenCredencialesEntrar()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  g->fillScreen(t.fondo);
  cabeceraDibujar("Credenciales", "< Ajustes");

  const int16_t y0 = CAB_ALTO + 8;
  tarjeta("Red WiFi (SSID)", redSsid(), y0);
  tarjeta("Contrasena", redPass(), y0 + TARJETA_H + 8);

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_center);
  g->setTextColor(t.textoTenue);
  g->drawString("Solo lectura. Se cambian con tools.py o el comando red.", ANCHO / 2, ALTO - 20);
  pantallaEmpujarTodo();
}

void screenCredencialesLoop()
{
  if (cabeceraAtras()) irAPantalla(SCREEN_AJUSTES);
}

void screenCredencialesSalir() {}
