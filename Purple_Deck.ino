/*
    Botonera 3x3 con iconos - WT32-SC01-Plus (ESP32-S3)
    Display/Touch : LovyanGFX  |  Board: ESP32S3 Dev Module

    Las imagenes se leen de LittleFS:  /boton1.png ... /boton9.png
    (carpeta "data" al lado de este .ino, subida con el plugin LittleFS)

    IMPORTANTE:
      - FS.h y LittleFS.h van ANTES de LovyanGFX.hpp, si no, drawPngFile()
        no ve el sistema de archivos y no compila.
      - Herramientas -> Partition Scheme: elegi un esquema con SPIFFS/FAT
        (p.ej. "Default 4MB with spiffs" o "16M Flash (3MB APP/9.9MB FATFS)")
      - Si falta algun PNG, ese boton simplemente se dibuja solo con texto.
*/

#include <FS.h>
#include <LittleFS.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "esp32-hal-psram.h"

// Portrait (fisico)
#define TFT_WIDTH   320
#define TFT_HEIGHT  480

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7796  _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_FT5x06  _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 40000000;
      cfg.pin_wr = 47;
      cfg.pin_rd = -1;
      cfg.pin_rs = 0;

      cfg.pin_d0 = 9;
      cfg.pin_d1 = 46;
      cfg.pin_d2 = 3;
      cfg.pin_d3 = 8;
      cfg.pin_d4 = 18;
      cfg.pin_d5 = 17;
      cfg.pin_d6 = 16;
      cfg.pin_d7 = 15;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs           =    -1;
      cfg.pin_rst          =     4;
      cfg.pin_busy         =    -1;

      cfg.panel_width      =   TFT_WIDTH;
      cfg.panel_height     =   TFT_HEIGHT;
      cfg.offset_x         =     0;
      cfg.offset_y         =     0;
      cfg.offset_rotation  =     0;
      cfg.dummy_read_pixel =     8;
      cfg.dummy_read_bits  =     1;
      cfg.readable         = false;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 45;
      cfg.invert = false;
      cfg.freq   = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;
      cfg.x_max      = 319;
      cfg.y_min      = 0;
      cfg.y_max      = 479;
      cfg.pin_int    = 7;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;

      cfg.i2c_port = 1;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda  = 6;
      cfg.pin_scl  = 5;
      cfg.freq = 400000;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

static LGFX lcd;
static LGFX_Sprite lienzo(&lcd);     // sprite reusable: dibuja un boton sin parpadeo

// ---------------- Paleta morada ----------------
static const uint16_t COL_FONDO    = lgfx::color565( 18,   7,  28);
static const uint16_t COL_BTN      = lgfx::color565(107,  33, 168);
static const uint16_t COL_BTN_ACT  = lgfx::color565(192, 132, 252);
static const uint16_t COL_BORDE    = lgfx::color565(167, 107, 232);
static const uint16_t COL_TXT      = lgfx::color565(255, 255, 255);
static const uint16_t COL_TXT_ACT  = lgfx::color565( 40,   8,  64);

// ---------------- Botonera ----------------
#define COLS 3
#define ROWS 3
#define NBTN (COLS * ROWS)
#define GAP  6
#define RAD  10

// 0 = icono arriba + texto abajo
// 1 = la imagen llena todo el boton y el texto va encima
#define IMAGEN_COMPLETA 0

#define ICON_TOP 5      // margen superior del icono dentro del boton
#define ICON_ALTO 54    // alto reservado para el icono

int16_t BW, BH;                       // tamano de cada boton (igual para todos)
int16_t OFFX, OFFY;                   // centrado de la rejilla en pantalla

struct Boton {
  int16_t x, y;
  uint8_t id;
  bool    tieneImg;
  char    ruta[16];     // "/boton9.png"
};

Boton botones[NBTN];

bool    fsOk       = false;
bool    usarSprite = false;
int8_t  btnActivo  = -1;
bool    dedoAbajo  = false;
uint32_t tPresion  = 0;
const uint32_t MOSTRAR_MS = 900;      // tiempo que queda marcado tras soltar

static int32_t tx, ty;

// ---------------------------------------------------------------------------
void crearBotones()
{
  const int16_t W = lcd.width();     // 480
  const int16_t H = lcd.height();    // 320

  const int16_t celdaW = W / COLS;   // 160
  const int16_t celdaH = H / ROWS;   // 106

  BW = celdaW - GAP;                 // 154
  BH = celdaH - GAP;                 // 100

  OFFX = (W - celdaW * COLS) / 2;    // sobrante repartido
  OFFY = (H - celdaH * ROWS) / 2;

  for (uint8_t f = 0; f < ROWS; f++) {
    for (uint8_t c = 0; c < COLS; c++) {
      uint8_t i = f * COLS + c;
      botones[i].x  = OFFX + c * celdaW + GAP / 2;
      botones[i].y  = OFFY + f * celdaH + GAP / 2;
      botones[i].id = i + 1;
      snprintf(botones[i].ruta, sizeof(botones[i].ruta), "/boton%u.png", i + 1);
      botones[i].tieneImg = fsOk && LittleFS.exists(botones[i].ruta);
    }
  }
}

// Dibuja el contenido del boton sobre 'g' con origen (ox, oy)
void pintarContenido(lgfx::LGFXBase *g, uint8_t i, bool presionado, int16_t ox, int16_t oy)
{
  const Boton &b = botones[i];
  uint16_t fondo = presionado ? COL_BTN_ACT : COL_BTN;
  uint16_t texto = presionado ? COL_TXT_ACT : COL_TXT;

  g->fillRoundRect(ox, oy, BW, BH, RAD, fondo);
  g->drawRoundRect(ox, oy, BW, BH, RAD, COL_BORDE);

  g->setTextColor(texto);
  g->setTextDatum(middle_center);

#if IMAGEN_COMPLETA
  if (b.tieneImg) {
    // la imagen ocupa todo el boton (se escala sola con scale = 0)
    g->drawPngFile(LittleFS, b.ruta, ox, oy, BW, BH, 0, 0, 0.0f, 0.0f,
                   datum_t::middle_center);
    g->drawRoundRect(ox, oy, BW, BH, RAD, COL_BORDE);
  }
  g->setFont(&fonts::Font2);
  g->drawString(presionado ? String("PRESIONADO") : (String("BOTON ") + b.id),
                ox + BW / 2, oy + BH - 14);

#else
  if (b.tieneImg) {
    // scale = 0 -> LovyanGFX escala la imagen para que entre en el area dada
    g->drawPngFile(LittleFS, b.ruta, ox + 8, oy + ICON_TOP, BW - 16, ICON_ALTO,
                   0, 0, 0.0f, 0.0f, datum_t::middle_center);

    g->setFont(&fonts::Font2);
    g->drawString(String("BOTON ") + b.id, ox + BW / 2, oy + ICON_TOP + ICON_ALTO + 12);
    if (presionado) {
      g->setFont(&fonts::Font0);
      g->drawString("PRESIONADO", ox + BW / 2, oy + BH - 10);
    }
  } else {
    // sin imagen: solo texto (como antes)
    g->setFont(&fonts::Font4);
    g->drawString(String("BOTON ") + b.id, ox + BW / 2, oy + BH / 2 - (presionado ? 14 : 0));
    if (presionado) {
      g->setFont(&fonts::Font2);
      g->drawString("PRESIONADO", ox + BW / 2, oy + BH / 2 + 14);
    }
  }
#endif
}

void dibujarBoton(uint8_t i, bool presionado)
{
  const Boton &b = botones[i];

  if (usarSprite) {
    lienzo.fillSprite(COL_FONDO);
    pintarContenido(&lienzo, i, presionado, 0, 0);
    lienzo.pushSprite(b.x, b.y);
  } else {
    pintarContenido(&lcd, i, presionado, b.x, b.y);
  }
}

void dibujarTodo()
{
  lcd.fillScreen(COL_FONDO);
  for (uint8_t i = 0; i < NBTN; i++) dibujarBoton(i, false);
}

int8_t botonEn(int32_t x, int32_t y)
{
  for (uint8_t i = 0; i < NBTN; i++) {
    const Boton &b = botones[i];
    if (x >= b.x && x < b.x + BW && y >= b.y && y < b.y + BH) return i;
  }
  return -1;
}

// ---------------------------------------------------------------------------
void setup(void)
{
  Serial.begin(115200);
  delay(200);

  lcd.init();
  lcd.setBrightness(200);
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

  lcd.fillScreen(COL_FONDO);
  lcd.setTextColor(COL_TXT, COL_FONDO);
  lcd.setFont(&fonts::Font2);
  lcd.setTextDatum(top_left);
  lcd.drawString("Montando LittleFS...", 10, 10);

  // false = no formatear si falla (asi no borra las imagenes ya subidas)
  fsOk = LittleFS.begin(false);
  if (!fsOk) {
    Serial.println("LittleFS no monto. Subiste la carpeta 'data'? Particion con SPIFFS?");
    lcd.drawString("LittleFS NO monto - se usa solo texto", 10, 30);
    delay(1500);
  } else {
    Serial.printf("LittleFS ok: %u / %u bytes usados\n",
                  (unsigned)LittleFS.usedBytes(), (unsigned)LittleFS.totalBytes());
  }

  crearBotones();

  // Sprite del tamano de un boton: primero en RAM interna, si no, en PSRAM
  lienzo.setColorDepth(16);
  usarSprite = lienzo.createSprite(BW, BH);
  if (!usarSprite && psramFound()) {
    lienzo.setPsram(true);
    usarSprite = lienzo.createSprite(BW, BH);
  }
  Serial.printf("Sprite %s (%dx%d)\n", usarSprite ? "ok" : "no disponible", BW, BH);

  for (uint8_t i = 0; i < NBTN; i++)
    if (!botones[i].tieneImg)
      Serial.printf("Falta %s\n", botones[i].ruta);

  dibujarTodo();
}

void loop()
{
  bool tocando = lcd.getTouch(&tx, &ty);

  if (tocando && !dedoAbajo) {                 // nueva pulsacion
    dedoAbajo = true;
    int8_t i = botonEn(tx, ty);

    if (i >= 0) {
      if (btnActivo >= 0 && btnActivo != i) dibujarBoton(btnActivo, false);
      btnActivo = i;
      tPresion  = millis();
      dibujarBoton(i, true);
      Serial.printf("Boton %d presionado\n", botones[i].id);

      // ---- aca va tu logica de cada boton ----
      // switch (botones[i].id) { case 1: ... }
    }
  }
  else if (!tocando && dedoAbajo) {            // se solto
    dedoAbajo = false;
  }

  if (btnActivo >= 0 && !dedoAbajo && (millis() - tPresion > MOSTRAR_MS)) {
    dibujarBoton(btnActivo, false);
    btnActivo = -1;
  }

  delay(10);
}
