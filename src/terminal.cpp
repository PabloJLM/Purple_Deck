#include "Drivers/terminal.h"
#include "Drivers/display.h"
#include "Drivers/tiempo.h"
#include "Drivers/temporizadores.h"
#include "Drivers/slots.h"
#include "Pantallas/screen_slides.h"
#include "config.h"
#include <SD.h>

static bool s_banner = false;

static void seccion(const char *titulo)
{
  Serial.println();
  Serial.print(F("== "));
  Serial.println(titulo);
}

static void fila(const char *clave, const String &valor)
{
  Serial.print(F("  "));
  Serial.print(clave);
  int pad = 16 - strlen(clave);
  for (int i = 0; i < pad; i++) Serial.print(' ');
  Serial.println(valor);
}

static String kb(uint64_t bytes)
{
  char b[24];
  snprintf(b, sizeof(b), "%lu KB", (unsigned long)(bytes / 1024UL));
  return String(b);
}

static String barra(uint64_t usado, uint64_t total)
{
  if (total == 0) return String("-");
  const uint8_t pct = (uint8_t)(usado * 100UL / total);
  char b[48];
  const uint8_t llenos = pct / 10;
  char dib[11];
  for (uint8_t i = 0; i < 10; i++) dib[i] = (i < llenos) ? '#' : '-';
  dib[10] = '\0';
  snprintf(b, sizeof(b), "[%s] %u%%", dib, pct);
  return String(b);
}

static void verMemoria()
{
  seccion("memoria (RAM)");
  fila("heap libre",  kb(ESP.getFreeHeap()));
  fila("heap total",  kb(ESP.getHeapSize()));
  fila("heap minimo", kb(ESP.getMinFreeHeap()));
  if (ESP.getPsramSize()) {
    fila("psram libre", kb(ESP.getFreePsram()));
    fila("psram total", kb(ESP.getPsramSize()));
  } else {
    fila("psram", "no detectada");
  }
}

static void verFlash()
{
  seccion("flash (programa)");
  const uint32_t prog = ESP.getSketchSize();
  const uint32_t libre = ESP.getFreeSketchSpace();
  fila("firmware",     kb(prog));
  fila("espacio libre", kb(libre));
  fila("chip flash",   kb(ESP.getFlashChipSize()));
}

static void verFS()
{
  seccion("LittleFS (iconos, sonidos)");
  if (!fsDisponible()) {
    Serial.println(F("  no disponible (sube 'data' con uploadfs)"));
    return;
  }
  const uint64_t usado = LittleFS.usedBytes();
  const uint64_t total = LittleFS.totalBytes();
  fila("usado",  kb(usado) + " de " + kb(total));
  fila("uso",    barra(usado, total));
}

static void verSD()
{
  seccion("tarjeta SD");
  if (!sdMontada()) {
    Serial.println(F("  sin tarjeta"));
    return;
  }
  const uint64_t total = SD.totalBytes();
  const uint64_t usado = SD.usedBytes();
  fila("tamano", kb(SD.cardSize()));
  fila("usado",  kb(usado) + " de " + kb(total));
  fila("uso",    barra(usado, total));
}

static void verTodo()
{
  verMemoria();
  verFlash();
  verFS();
  verSD();
  Serial.println();
}

static void verUptime()
{
  const unsigned long s = millis() / 1000UL;
  char b[32];
  snprintf(b, sizeof(b), "%luh %02lum %02lus", s / 3600UL, (s % 3600UL) / 60UL, s % 60UL);
  seccion("uptime");
  fila("activo desde", String(b));
}

static void verInfo()
{
  seccion("purple deck");
  fila("proyecto", "Purple_Deck");
  fila("mcu",      "ESP32-S3");
  fila("pantalla", "ST7796 480x320");
  fila("lab",      "Tesla Lab");
}

static void prompt()
{
  Serial.print(F("deck:~$ "));
}

static void ayuda()
{
  seccion("comandos");
  Serial.println(F("  help       esta ayuda"));
  Serial.println(F("  mem        memoria RAM / PSRAM libre"));
  Serial.println(F("  flash      espacio del programa en la flash"));
  Serial.println(F("  fs         espacio de LittleFS (iconos, sonidos)"));
  Serial.println(F("  sd         espacio de la tarjeta SD"));
  Serial.println(F("  df         los 4 anteriores juntos"));
  Serial.println(F("  info       datos del deck"));
  Serial.println(F("  uptime     tiempo desde el ultimo reinicio"));
  Serial.println(F("  slot       espacios editables del menu (escribe 'slot help')"));
  Serial.println(F("  reboot     reinicia el deck"));
  Serial.println(F("  clear      limpia la pantalla del monitor"));
}

static void procesar(String cmd)
{
  cmd.trim();
  if      (cmd.length() == 0) { }
  else if (cmd == "help")     ayuda();
  else if (cmd == "mem")      verMemoria();
  else if (cmd == "flash")    verFlash();
  else if (cmd == "fs")       verFS();
  else if (cmd == "sd")       verSD();
  else if (cmd == "df")       verTodo();
  else if (cmd == "info")     verInfo();
  else if (cmd == "uptime")   verUptime();
  else if (slotsComando(cmd)) { }
  else if (cmd == "reboot")   { Serial.println(F("  reiniciando...")); delay(150); ESP.restart(); }
  else if (cmd == "clear" || cmd == "cls") Serial.print(F("\033[2J\033[H"));
  else {
    Serial.print(F("  comando no encontrado: "));
    Serial.println(cmd);
    Serial.println(F("  escribe 'help' para ver los comandos"));
  }
  Serial.println();
  prompt();
}

static void banner()
{
  Serial.println();
  Serial.println(F("  ---- Purple Deck ----"));
  Serial.println(F("  Escribe 'help' para ver los comandos."));
  Serial.println();
  prompt();
}

void terminalIniciar()
{
  s_banner = false;
}

void terminalLoop()
{
  if (!s_banner) { banner(); s_banner = true; }

  static String buffer;
  while (Serial.available() > 0) {
    const char c = (char)Serial.read();

    if (c == '\n' || c == '\r') {
      if (buffer.length() > 0) {
        Serial.println();
        procesar(buffer);
        buffer = "";
      }
    } else if (c == 8 || c == 127) {
      if (buffer.length() > 0) {
        buffer.remove(buffer.length() - 1);
        Serial.write(8); Serial.write(' '); Serial.write(8);
      }
    } else if (c >= 32 && c <= 126) {
      buffer += c;
      Serial.write(c);
    }
  }
}
