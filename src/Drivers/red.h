#ifndef PURPLE_DECK_DRIVERS_RED_H_
#define PURPLE_DECK_DRIVERS_RED_H_
// ---------------------------------------------------------------------------
//  Drivers/red.h - credenciales de WiFi y FTP guardadas en la NVS
//
//  Se cambian por la terminal serie (comando 'red', ver 'red help') o con
//  tools.py, sin recompilar ni reflashear. Si la NVS esta vacia se usan los
//  valores de secrets.h (WIFI_SSID / WIFI_PASS) como valores iniciales.
// ---------------------------------------------------------------------------
#include <Arduino.h>

const char *redSsid();
const char *redPass();
const char *redFtpUser();
const char *redFtpPass();

bool redComando(const String &linea);   // true si la linea era un comando 'red'

#endif  // PURPLE_DECK_DRIVERS_RED_H_
