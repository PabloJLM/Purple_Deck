#include "Drivers/audio.h"
#include "Drivers/display.h"
#include "config.h"

#include <ESP_I2S.h>

#define BLOQUE 512

static I2SClass      s_i2s;
static QueueHandle_t s_cola = nullptr;
static uint32_t      s_tasa = 0;

static bool buscarChunk(File &f, const char *id, uint32_t &largo)
{
  char cab[4];
  while (f.read((uint8_t *)cab, 4) == 4 && f.read((uint8_t *)&largo, 4) == 4) {
    if (memcmp(cab, id, 4) == 0) return true;
    f.seek(f.position() + largo + (largo & 1));
  }
  return false;
}

static void tocar(const char *ruta)
{
  File f = LittleFS.open(ruta, "r");
  if (!f) { Serial.printf("[audio] no existe %s\n", ruta); return; }

  char riff[12];
  uint32_t largo = 0;
  uint16_t fmt[8];
  if (f.read((uint8_t *)riff, 12) != 12 || memcmp(riff, "RIFF", 4) || memcmp(riff + 8, "WAVE", 4) ||
      !buscarChunk(f, "fmt ", largo) || largo < 16 || f.read((uint8_t *)fmt, 16) != 16) {
    Serial.printf("[audio] %s no es WAV\n", ruta);
    f.close();
    return;
  }
  f.seek(f.position() + largo - 16);

  const uint16_t canales = fmt[1];
  const uint32_t tasa    = fmt[2] | ((uint32_t)fmt[3] << 16);
  const uint16_t bits    = fmt[7];
  if (fmt[0] != 1 || canales != 1 || bits != 16 || !buscarChunk(f, "data", largo)) {
    Serial.printf("[audio] %s debe ser PCM 16 bit mono\n", ruta);
    f.close();
    return;
  }

  if (tasa != s_tasa) {
    s_i2s.configureTX(tasa, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_BOTH);
    s_tasa = tasa;
  }

  int16_t buf[BLOQUE];
  const int32_t ganancia = (int32_t)volumen * volumen;
  while (largo > 0) {
    const size_t n = f.read((uint8_t *)buf, min<uint32_t>(sizeof(buf), largo));
    if (n == 0) break;
    largo -= n;
    for (size_t i = 0; i < n / 2; i++) buf[i] = (int16_t)((int32_t)buf[i] * ganancia / 10000);
    s_i2s.write((uint8_t *)buf, n);
  }
  f.close();
}

static void tarea(void *)
{
  const char *ruta;
  for (;;) {
    if (xQueueReceive(s_cola, &ruta, portMAX_DELAY) == pdTRUE && volumen > 0) tocar(ruta);
  }
}

bool audioIniciar()
{
  s_i2s.setPins(AUDIO_BCLK, AUDIO_LRCK, AUDIO_DOUT);
  if (!s_i2s.begin(I2S_MODE_STD, AUDIO_TASA, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_BOTH)) {
    Serial.println("[audio] I2S no arranco");
    return false;
  }
  s_tasa = AUDIO_TASA;
  s_cola = xQueueCreate(1, sizeof(const char *));
  xTaskCreatePinnedToCore(tarea, "audio", 4096, nullptr, 2, nullptr, 0);
  Serial.println("[audio] listo");
  return true;
}

void audioReproducir(const char *ruta)
{
  if (s_cola && fsDisponible()) xQueueOverwrite(s_cola, &ruta);
}

void audioNotificacion(NotifTipo tipo)
{
  switch (tipo) {
    case NOTIF_ALERTA:  audioReproducir(SONIDO_ALERTA);  break;
    case NOTIF_MENSAJE: audioReproducir(SONIDO_MENSAJE); break;
    default:            audioReproducir(SONIDO_INFO);    break;
  }
}
