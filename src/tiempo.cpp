#include "Drivers/tiempo.h"
#include "config.h"
#include "secrets.h"

#include <WiFi.h>

const char *DIA_CORTO[7] = { "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB" };
const char *DIA_LARGO[7] = { "Domingo", "Lunes", "Martes", "Miercoles",
                             "Jueves", "Viernes", "Sabado" };
const char *MES_CORTO[12] = { "ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
                              "JUL", "AGO", "SEP", "OCT", "NOV", "DIC" };

static bool     s_ntpPedido = false;
static bool     s_sinc      = false;
static uint32_t s_ultimoIntento = 0;

#define REINTENTO_MS 15000UL

void tiempoIniciar()
{
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  s_ultimoIntento = millis();
  Serial.printf("[wifi] conectando a %s\n", WIFI_SSID);
}

bool wifiConectado() { return WiFi.status() == WL_CONNECTED; }

bool tiempoSincronizado() { return s_sinc; }

void tiempoLoop()
{
  if (!wifiConectado()) {
    s_ntpPedido = false;
    if (millis() - s_ultimoIntento > REINTENTO_MS) {
      s_ultimoIntento = millis();
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      Serial.println("[wifi] reintentando");
    }
    return;
  }

  if (!s_ntpPedido) {
    s_ntpPedido = true;
    configTzTime(TZ_LOCAL, NTP_1, NTP_2);
    Serial.printf("[wifi] conectado, IP %s - pidiendo NTP\n",
                  WiFi.localIP().toString().c_str());
  }

  if (!s_sinc) {
    struct tm t;
    if (getLocalTime(&t, 0) && t.tm_year > (2020 - 1900)) {
      s_sinc = true;
      Serial.printf("[ntp] hora: %02d:%02d:%02d %02d/%02d/%04d\n",
                    t.tm_hour, t.tm_min, t.tm_sec,
                    t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
    }
  }
}

bool tiempoObtener(struct tm &t)
{
  if (!s_sinc) return false;
  return getLocalTime(&t, 0);
}
