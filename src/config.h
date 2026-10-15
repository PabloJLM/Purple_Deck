#ifndef PURPLE_DECK_CONFIG_H_
#define PURPLE_DECK_CONFIG_H_
// ---------------------------------------------------------------------------
//  config.h - constantes globales y enum de pantallas (estilo camioneta)
// ---------------------------------------------------------------------------
#include <Arduino.h>

// ---- Pantalla fisica (horizontal) ----
#define ANCHO 480
#define ALTO  320

// ---- Rejilla de botones (menu principal y submenus) ----
#define GAP       6      // separacion entre botones (px)
#define RAD       10     // radio de las esquinas

// ---- Idle / bloqueo ----
// Valores de arranque: en Ajustes se pueden cambiar mientras el deck esta
// encendido (no se guardan al reiniciar).
#define IDLE_TIMEOUT_MS   10000UL    // sin tocar -> entra al idle
#define BRILLO_NORMAL     200
#define BRILLO_IDLE       110

// ---- Reloj ----
enum RelojModo : uint8_t { RELOJ_ANALOGICO = 0, RELOJ_DIGITAL = 1 };
#define RELOJ_POR_DEFECTO RELOJ_DIGITAL   // con que arranca el idle

// ---- Zona horaria (Guatemala, sin horario de verano) ----
#define TZ_LOCAL "CST6"
#define NTP_1 "pool.ntp.org"
#define NTP_2 "time.nist.gov"

// ---- Pantallas ----
enum Screen : uint8_t {
  SCREEN_MENU = 0,       // menu principal
  SCREEN_MACROS,
  SCREEN_SOLIDWORKS,
  SCREEN_LABVIEW,
  SCREEN_GIT,            // Git + VS Code
  SCREEN_AJUSTES,
  SCREEN_ARDUINO,
  SCREEN_IDLE            // bloqueo: reloj + notificaciones
};

extern Screen pantallaActual;
void irAPantalla(Screen s);
void salirDeIdle();      // vuelve a la pantalla donde estaba antes del idle

// ---- Estado del toque, compartido por todas las pantallas ----
extern bool    toqueNuevo;    // true solo en el ciclo del flanco de bajada
extern bool    toqueActivo;   // dedo abajo ahora
extern int32_t toqueX, toqueY;

// ---- Ajustes en vivo (se cambian desde la pantalla Ajustes) ----
extern uint32_t idleTimeoutMs;   // arranca en IDLE_TIMEOUT_MS
extern uint8_t  brilloNormal;    // arranca en BRILLO_NORMAL

#endif  // PURPLE_DECK_CONFIG_H_
