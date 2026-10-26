#include "Drivers/display.h"
#include "config.h"

LGFX lcd;

static bool s_fsOk = false;
static LGFX_Sprite s_fb(&lcd);
static bool s_fbOk = false;

bool fsDisponible() { return s_fsOk; }

lgfx::LGFXBase *pantalla()
{
  return s_fbOk ? (lgfx::LGFXBase *)&s_fb : (lgfx::LGFXBase *)&lcd;
}

bool pantallaDoble() { return s_fbOk; }

void pantallaEmpujar(int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (!s_fbOk) return;
  lcd.setClipRect(x, y, w, h);
  s_fb.pushSprite(&lcd, 0, 0);
  lcd.clearClipRect();
}

void pantallaEmpujarTodo() { pantallaEmpujar(0, 0, ANCHO, ALTO); }

bool displayIniciar()
{
  lcd.init();
  lcd.setBrightness(BRILLO_NORMAL);

  // Horizontal: 480 x 320
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);
  lcd.setRotation(lcd.getRotation() ^ 2);

  if (psramFound()) {
    s_fb.setColorDepth(16);
    s_fb.setPsram(true);
    s_fbOk = s_fb.createSprite(ANCHO, ALTO);
  }
  Serial.printf("[gfx] buffer de pantalla completa: %s\n", s_fbOk ? "ok (PSRAM)" : "no disponible");

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
