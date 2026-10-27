#include "Drivers/red.h"
#include "Drivers/slots_util.h"
#include "Drivers/tiempo.h"
#include "secrets.h"

#include <Preferences.h>
#include <WiFi.h>

// Valores iniciales cuando la NVS esta vacia
#ifndef FTP_USER
#define FTP_USER "pablo"
#endif
#ifndef FTP_PASS
#define FTP_PASS "purpledeck"
#endif

#define L_SSID 32
#define L_PASS 63
#define L_FTP  31

static char s_ssid[L_SSID + 1], s_pass[L_PASS + 1];
static char s_fu[L_FTP + 1], s_fp[L_FTP + 1];
static bool s_cargado = false;

static void cargar()
{
  if (s_cargado) return;
  s_cargado = true;
  Preferences p;
  p.begin("red", true);
  String a = p.getString("ssid", WIFI_SSID);
  String b = p.getString("pass", WIFI_PASS);
  String c = p.getString("fu", FTP_USER);
  String d = p.getString("fp", FTP_PASS);
  p.end();
  strlcpy(s_ssid, a.c_str(), sizeof(s_ssid));
  strlcpy(s_pass, b.c_str(), sizeof(s_pass));
  strlcpy(s_fu,   c.c_str(), sizeof(s_fu));
  strlcpy(s_fp,   d.c_str(), sizeof(s_fp));
}

const char *redSsid()    { cargar(); return s_ssid; }
const char *redPass()    { cargar(); return s_pass; }
const char *redFtpUser() { cargar(); return s_fu; }
const char *redFtpPass() { cargar(); return s_fp; }

static void ok()               { Serial.println(F("OK")); }
static void err(const char *m) { Serial.print(F("ERR ")); Serial.println(m); }

static void ayuda()
{
  Serial.println(F("  red get                          muestra WiFi y FTP"));
  Serial.println(F("  red wifi \"ssid\" \"clave\"          cambia el WiFi y reconecta"));
  Serial.println(F("  red ftp \"usuario\" \"clave\"        cambia el usuario y clave del FTP"));
  Serial.println(F("  (el FTP nuevo se usa la proxima vez que abras la pantalla FTP)"));
}

static void ver()
{
  cargar();
  Serial.printf("RED wifi|%s|%s\n", s_ssid, s_pass);
  Serial.printf("RED ftp|%s|%s\n", s_fu, s_fp);
  ok();
}

static bool texto_ok(const char *s, size_t max)
{
  if (strlen(s) > max) return false;
  for (; *s; s++) if (*s < 32 || *s > 126) return false;
  return true;
}

bool redComando(const String &linea)
{
  if (!(linea == "red" || linea.startsWith("red "))) return false;

  char buf[200];
  if (linea.length() >= sizeof(buf)) { err("linea demasiado larga"); return true; }
  strlcpy(buf, linea.c_str(), sizeof(buf));

  char *t[5];
  const int n = slotTokenizar(buf, t, 5);
  if (n == -1) { err("comilla sin cerrar"); return true; }
  if (n < 0)   { err("demasiados argumentos"); return true; }

  if (n == 1 || !strcmp(t[1], "help")) { ayuda(); return true; }
  if (!strcmp(t[1], "get")) { ver(); return true; }

  const bool wifi = !strcmp(t[1], "wifi");
  const bool ftp  = !strcmp(t[1], "ftp");
  if (!wifi && !ftp) { err("subcomando desconocido (escribe 'red help')"); return true; }
  if (n != 4) { err(wifi ? "uso: red wifi \"ssid\" \"clave\"" : "uso: red ftp \"usuario\" \"clave\""); return true; }

  cargar();
  if (wifi) {
    if (!t[2][0])                        { err("el SSID no puede estar vacio"); return true; }
    if (!texto_ok(t[2], L_SSID))         { err("SSID invalido (max 32, ASCII)"); return true; }
    if (t[3][0] && strlen(t[3]) < 8)     { err("la clave WPA2 necesita 8 o mas caracteres"); return true; }
    if (!texto_ok(t[3], L_PASS))         { err("clave invalida (max 63, ASCII)"); return true; }
    strlcpy(s_ssid, t[2], sizeof(s_ssid));
    strlcpy(s_pass, t[3], sizeof(s_pass));
    Preferences p;
    p.begin("red", false);
    p.putString("ssid", s_ssid);
    p.putString("pass", s_pass);
    p.end();
    tiempoReconectar();
  } else {
    if (!t[2][0] || !t[3][0])            { err("usuario y clave no pueden estar vacios"); return true; }
    if (!texto_ok(t[2], L_FTP) || !texto_ok(t[3], L_FTP)) { err("invalido (max 31, ASCII)"); return true; }
    strlcpy(s_fu, t[2], sizeof(s_fu));
    strlcpy(s_fp, t[3], sizeof(s_fp));
    Preferences p;
    p.begin("red", false);
    p.putString("fu", s_fu);
    p.putString("fp", s_fp);
    p.end();
  }
  ok();
  return true;
}
