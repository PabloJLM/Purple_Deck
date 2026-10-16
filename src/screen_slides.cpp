#include "Pantallas/screen_slides.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "config.h"

#include <SPI.h>
#include <SD.h>

#define SLIDES_MAX 40
#define RUTA_LEN   64

static char     s_rutas[SLIDES_MAX][RUTA_LEN];
static uint8_t  s_n    = 0;
static fs::FS  *s_fs   = nullptr;
static bool     s_sdOk = false;
static uint8_t  s_idx  = 0;
static uint32_t s_t    = 0;
static bool     s_mostrada = false;

static bool termina(const char *n, const char *ext)
{
  const size_t a = strlen(n), b = strlen(ext);
  return a >= b && strcasecmp(n + a - b, ext) == 0;
}

static bool esImagen(const char *n)
{
  return termina(n, ".jpg") || termina(n, ".jpeg") || termina(n, ".png");
}

static uint8_t listar(fs::FS &fs)
{
  File dir = fs.open(SLIDES_DIR);
  if (!dir || !dir.isDirectory()) return 0;

  uint8_t n = 0;
  File f = dir.openNextFile();
  while (f && n < SLIDES_MAX) {
    if (!f.isDirectory() && esImagen(f.name())) {
      snprintf(s_rutas[n], RUTA_LEN, "%s/%s", SLIDES_DIR, f.name());
      n++;
    }
    f.close();
    f = dir.openNextFile();
  }
  dir.close();
  return n;
}

static bool montarSD()
{
  if (s_sdOk) return true;
  SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  s_sdOk = SD.begin(SD_CS, SPI, 20000000);
  Serial.printf("[sd] %s\n", s_sdOk ? "montada" : "sin tarjeta");
  return s_sdOk;
}

void slidesEscanear()
{
  s_n = 0;
  s_fs = nullptr;
  s_idx = 0;

  if (montarSD() && (s_n = listar(SD)) > 0) {
    s_fs = &SD;
  } else if (fsDisponible() && (s_n = listar(LittleFS)) > 0) {
    s_fs = &LittleFS;
  }
  Serial.printf("[slides] %u imagenes en %s\n", (unsigned)s_n, slidesOrigen() ? slidesOrigen() : "-");
}

void slidesIniciar() { slidesEscanear(); }

uint8_t slidesCantidad() { return s_n; }

const char *slidesOrigen()
{
  if (!s_fs) return nullptr;
  return s_fs == &SD ? "SD" : "Flash";
}

static void mostrar()
{
  const char *r = s_rutas[s_idx];
  lcd.fillScreen(TFT_BLACK);
  if (termina(r, ".png"))
    lcd.drawPngFile(*s_fs, r, 0, 0, ANCHO, ALTO, 0, 0, 0.0f, 0.0f, datum_t::middle_center);
  else
    lcd.drawJpgFile(*s_fs, r, 0, 0, ANCHO, ALTO, 0, 0, 0.0f, 0.0f, datum_t::middle_center);
}

void screenSlidesEntrar()
{
  s_mostrada = false;
  if (!s_n) {
    const Tema &t = T();
    lcd.fillScreen(t.fondo);
    lcd.setTextSize(1.0f);
    lcd.setFont(&fonts::Font4);
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(t.textoTenue);
    lcd.drawString("Sin imagenes en " SLIDES_DIR, ANCHO / 2, ALTO / 2);
  }
}

void screenSlidesLoop()
{
  if (deslizoDerecha()) { salirDeIdle(); return; }
  if (!s_n) return;

  if (!s_mostrada || millis() - s_t > SLIDE_MS) {
    mostrar();
    s_t = millis();
    s_mostrada = true;
    s_idx = (s_idx + 1) % s_n;
  }
}

void screenSlidesSalir() {}
