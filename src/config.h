#ifndef PURPLE_DECK_CONFIG_H_
#define PURPLE_DECK_CONFIG_H_
#include <Arduino.h>

#define ANCHO 480
#define ALTO  320

#define GAP 6
#define RAD 10

#define IDLE_TIMEOUT_MS 90000UL   // 1 min 30 s
#define BRILLO_NORMAL   200
#define BRILLO_IDLE     110
#define BRILLO_MIN_PCT  5
#define REPOSO_MIN_S    5
#define REPOSO_MAX_S    300

enum RelojModo : uint8_t { RELOJ_ANALOGICO = 0, RELOJ_DIGITAL = 1 };
#define RELOJ_POR_DEFECTO RELOJ_DIGITAL

enum BloqueoModo : uint8_t { BLOQUEO_RELOJ = 0, BLOQUEO_SLIDES = 1 };
#define SLIDES_DIR "/slides"
#define SLIDE_MS   8000UL

#define SD_CS   41
#define SD_MOSI 40
#define SD_CLK  39
#define SD_MISO 38

#define AUDIO_LRCK 35
#define AUDIO_BCLK 36
#define AUDIO_DOUT 37
#define AUDIO_TASA 22050
#define VOLUMEN_INICIAL 60
#define SONIDO_INFO    "/sonidos/info.wav"
#define SONIDO_MENSAJE "/sonidos/mensaje.wav"
#define SONIDO_ALERTA  "/sonidos/alerta.wav"
#define SONIDO_VOLUMEN "/sonidos/volumen.wav"

#define POMO_FOCO_MIN  25
#define POMO_CORTO_MIN 5
#define POMO_LARGO_MIN 15
#define POMO_CICLOS    4
#define NUM_ALARMAS    4

#define APP_SPOTIFY    "spotify"
#define APP_NAVEGADOR  "brave"

// SolidWorks no trae atajo de fabrica para Extruir ni Sketch: el deck manda
// Ctrl+Shift+<tecla> y hay que asignarlo en SolidWorks
// (Herramientas > Personalizar > Teclado).

#define TZ_LOCAL "CST6"
#define NTP_1 "pool.ntp.org"
#define NTP_2 "time.nist.gov"

#define TOQUE_TOL    15
#define DESLIZAR_MIN 90

enum Screen : uint8_t {
  SCREEN_MENU = 0,
  SCREEN_MACROS,
  SCREEN_SOLIDWORKS,
  SCREEN_SW_ROTAR,
  SCREEN_SW_MOVER,
  SCREEN_LABVIEW,
  SCREEN_GIT,
  SCREEN_BLENDER,
  SCREEN_CANDLE,
  SCREEN_CANDLE_MOVER,
  SCREEN_AJUSTES,
  SCREEN_BRILLO,
  SCREEN_REPOSO,
  SCREEN_VOLUMEN,
  SCREEN_TEMAS,
  SCREEN_CREDENCIALES,
  SCREEN_SISTEMA,
  SCREEN_FTP,
  SCREEN_BLUETOOTH,
  SCREEN_POMODORO,
  SCREEN_RELOJ,
  SCREEN_IDLE,
  SCREEN_SLIDES
};

extern Screen pantallaActual;
void irAPantalla(Screen s);
void salirDeIdle();
void pedirBloqueo(uint8_t modo);

// toquePropio: el gesto empezo en la pantalla actual (no viene de la anterior)
extern bool    toqueNuevo;
extern bool    toqueActivo;
extern bool    toqueSuelto;
extern bool    toquePropio;
extern int32_t toqueX, toqueY;
extern int32_t toqueX0, toqueY0;

inline int32_t absI(int32_t v) { return v < 0 ? -v : v; }

inline bool fueToque()
{
  return toqueSuelto && absI(toqueX - toqueX0) < TOQUE_TOL && absI(toqueY - toqueY0) < TOQUE_TOL;
}

inline bool deslizoDerecha()
{
  const int32_t dx = toqueX - toqueX0;
  return toqueSuelto && dx > DESLIZAR_MIN && dx > 2 * absI(toqueY - toqueY0);
}

extern uint32_t idleTimeoutMs;
extern uint8_t  brilloNormal;
extern uint8_t  modoBloqueo;
extern uint8_t  volumen;

#endif  // PURPLE_DECK_CONFIG_H_
