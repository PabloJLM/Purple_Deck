#include "Drivers/display.h"
#include "config.h"

LGFX lcd;

static bool s_fsOk = false;

bool fsDisponible() { return s_fsOk; }

bool displayIniciar()
{
  lcd.init();
  lcd.setBrightness(BRILLO_NORMAL);

  // Horizontal: 480 x 320
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

  // false = no formatear si falla, para no borrar las imagenes ya subidas
  s_fsOk = LittleFS.begin(false);
  if (s_fsOk) {
    Serial.printf("[fs] LittleFS ok: %u / %u bytes\n",
                  (unsigned)LittleFS.usedBytes(), (unsigned)LittleFS.totalBytes());
  } else {
    Serial.println("[fs] LittleFS no monto (particion sin SPIFFS o 'data' sin subir)");
  }
  return s_fsOk;
}
