#include "Pantallas/screen_ftp.h"
#include "Pantallas/screen_slides.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/ui.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Drivers/tiempo.h"
#include "config.h"
#include "Drivers/red.h"

#include <WiFi.h>
#include <SimpleFTPServer.h>

#define TARJETA_X  GAP
#define TARJETA_W  (ANCHO - 2 * GAP)
#define TARJETA_H  56
#define TEXTO_MAX  (TARJETA_W - 32)

#define BOTON_H 48
#define BOTON_Y (CAB_ALTO + 6 + 3 * (TARJETA_H + 6) + 20)

static FtpServer s_ftp;
static bool      s_activo   = false;
static bool      s_wifiPrev = false;

static void iniciarFtp()
{
  if (s_activo) return;
  sdMontada();  // asegura que la SD este montada antes de servirla por FTP
  s_ftp.begin(redFtpUser(), redFtpPass(), "Purple Deck FTP");
  s_activo = true;
  Serial.printf("[ftp] servidor iniciado (usuario %s)\n", redFtpUser());
}

static void detenerFtp()
{
  if (!s_activo) return;
  s_ftp.end();
  s_activo = false;
  Serial.println("[ftp] servidor detenido");
}

bool ftpActivo() { return s_activo; }

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
  const bool wifi = wifiConectado();
  s_wifiPrev = wifi;

  g->fillRect(0, CAB_ALTO, ANCHO, ALTO - CAB_ALTO, t.fondo);

  const int16_t y0 = CAB_ALTO + 6;

  char direccion[48];
  if (wifi) snprintf(direccion, sizeof(direccion), "ftp://%s", WiFi.localIP().toString().c_str());
  else      snprintf(direccion, sizeof(direccion), "Sin conexion WiFi");
  tarjeta("Direccion", direccion, y0);
  tarjeta("Usuario",   redFtpUser(),  y0 + (TARJETA_H + 6));
  tarjeta("Clave",     redFtpPass(),  y0 + 2 * (TARJETA_H + 6));

  const char *estado;
  if (!wifi)        estado = "Esperando WiFi...";
  else if (s_activo) estado = sdMontada() ? "Activo - conectate con un cliente FTP"
                                          : "Activo - SIN tarjeta SD";
  else               estado = "Detenido";

  g->setTextSize(1.0f);
  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_center);
  g->setTextColor(t.textoTenue);
  g->drawString(estado, ANCHO / 2, CAB_ALTO + 6 + 3 * (TARJETA_H + 6) + 6);

  const char *texBoton = !wifi ? "Esperando WiFi" : (s_activo ? "Detener FTP" : "Iniciar FTP");
  const uint16_t relleno = (!wifi) ? t.superficieAlt : (s_activo ? t.superficie : t.acento);
  const uint16_t colorTxt = (!wifi) ? t.textoTenue : (s_activo ? t.texto : t.textoSobreAcento);
  botonDibujar(g, TARJETA_X, BOTON_Y, TARJETA_W, BOTON_H, texBoton, relleno, colorTxt, &fonts::Font4);

  pantallaEmpujar(0, CAB_ALTO, ANCHO, ALTO - CAB_ALTO);
}

void screenFtpEntrar()
{
  pantalla()->fillScreen(T().fondo);
  cabeceraDibujar("Servidor FTP", "< Ajustes");
  dibujar();
  pantallaEmpujarTodo();
}

void screenFtpLoop()
{
  if (cabeceraAtras()) { irAPantalla(SCREEN_AJUSTES); return; }

  if (s_activo) s_ftp.handleFTP();

  if (toqueEn(TARJETA_X, BOTON_Y, TARJETA_W, BOTON_H) && wifiConectado()) {
    if (s_activo) detenerFtp();
    else          iniciarFtp();
    dibujar();
    return;
  }

  if (wifiConectado() != s_wifiPrev) dibujar();
}

void screenFtpSalir()
{
  detenerFtp();
}
