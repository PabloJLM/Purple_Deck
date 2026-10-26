#include "Pantallas/screen_credenciales.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "config.h"
#include "secrets.h"

#define TARJETA_X  GAP
#define TARJETA_W  (ANCHO - 2 * GAP)
#define TARJETA_H  84
#define TEXTO_MAX  (TARJETA_W - 32)

static void valor(const char *texto, int16_t y)
{
  const Tema &t = T();
  char b[72];
  strncpy(b, texto, sizeof(b) - 1);
  b[sizeof(b) - 1] = '\0';

  lcd.setTextSize(1.0f);
  lcd.setFont(&fonts::Font4);
  if (lcd.textWidth(b) > TEXTO_MAX) lcd.setFont(&fonts::Font2);

  size_t n = strlen(b);
  bool cortado = false;
  while (n > 3 && lcd.textWidth(b) > TEXTO_MAX) {
    b[--n] = '\0';
    cortado = true;
  }
  if (cortado) { b[n - 1] = b[n - 2] = b[n - 3] = '.'; }

  lcd.setTextDatum(middle_left);
  lcd.setTextColor(t.texto);
  lcd.drawString(b, TARJETA_X + 16, y);
}

static void tarjeta(const char *etiqueta, const char *texto, int16_t y)
{
  const Tema &t = T();
  lcd.fillRoundRect(TARJETA_X, y, TARJETA_W, TARJETA_H, RAD, t.superficie);
  lcd.drawRoundRect(TARJETA_X, y, TARJETA_W, TARJETA_H, RAD, t.borde);

  lcd.setTextSize(1.0f);
  lcd.setFont(&fonts::Font2);
  lcd.setTextDatum(top_left);
  lcd.setTextColor(t.acento);
  lcd.drawString(etiqueta, TARJETA_X + 16, y + 10);

  valor(texto, y + 54);
}

void screenCredencialesEntrar()
{
  const Tema &t = T();
  lcd.fillScreen(t.fondo);
  cabeceraDibujar("Credenciales", "< Ajustes");

  const int16_t y0 = CAB_ALTO + 8;
  tarjeta("Red WiFi (SSID)", WIFI_SSID, y0);
  tarjeta("Contrasena", WIFI_PASS, y0 + TARJETA_H + 8);

  lcd.setTextSize(1.0f);
  lcd.setFont(&fonts::Font2);
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(t.textoTenue);
  lcd.drawString("Solo lectura. Se cambian con configurar.py y recompilando.", ANCHO / 2, ALTO - 20);
}

void screenCredencialesLoop()
{
  if (cabeceraAtras()) irAPantalla(SCREEN_AJUSTES);
}

void screenCredencialesSalir() {}
