#include "config.h"
#include "Drivers/display.h"
#include "Drivers/tiempo.h"
#include "Drivers/notificaciones.h"
#include "Estaticos/temas.h"
#include "Pantallas/screen_menu.h"
#include "Pantallas/screen_macros.h"
#include "Pantallas/screen_solidworks.h"
#include "Pantallas/screen_labview.h"
#include "Pantallas/screen_git.h"
#include "Pantallas/screen_ajustes.h"
#include "Pantallas/screen_arduino.h"
#include "Pantallas/screen_idle.h"

// ---- Estado global declarado en config.h ----
Screen   pantallaActual = SCREEN_MENU;
bool     toqueNuevo  = false;
bool     toqueActivo = false;
int32_t  toqueX = 0, toqueY = 0;
uint32_t idleTimeoutMs = IDLE_TIMEOUT_MS;
uint8_t  brilloNormal  = BRILLO_NORMAL;

static uint32_t s_ultimaActividad = 0;
static Screen   s_previaIdle      = SCREEN_MENU;   // donde estaba al dormirse

// ---------------------------------------------------------------------------
static void salirDe(Screen s)
{
  switch (s) {
    case SCREEN_MENU:       screenMenuSalir();       break;
    case SCREEN_MACROS:     screenMacrosSalir();     break;
    case SCREEN_SOLIDWORKS: screenSolidworksSalir(); break;
    case SCREEN_LABVIEW:    screenLabviewSalir();    break;
    case SCREEN_GIT:        screenGitSalir();        break;
    case SCREEN_AJUSTES:    screenAjustesSalir();    break;
    case SCREEN_ARDUINO:    screenArduinoSalir();    break;
    case SCREEN_IDLE:       screenIdleSalir();       break;
  }
}

static void entrarEn(Screen s)
{
  switch (s) {
    case SCREEN_MENU:       screenMenuEntrar();       break;
    case SCREEN_MACROS:     screenMacrosEntrar();     break;
    case SCREEN_SOLIDWORKS: screenSolidworksEntrar(); break;
    case SCREEN_LABVIEW:    screenLabviewEntrar();    break;
    case SCREEN_GIT:        screenGitEntrar();        break;
    case SCREEN_AJUSTES:    screenAjustesEntrar();    break;
    case SCREEN_ARDUINO:    screenArduinoEntrar();    break;
    case SCREEN_IDLE:       screenIdleEntrar();       break;
  }
}

void irAPantalla(Screen s)
{
  if (s == pantallaActual) return;

  salirDe(pantallaActual);

  pantallaActual    = s;
  toqueNuevo        = false;        // el toque que cambio de pantalla no sigue
  s_ultimaActividad = millis();

  entrarEn(pantallaActual);
}

void salirDeIdle()
{
  irAPantalla(s_previaIdle);
}

// ---------------------------------------------------------------------------
static void leerToque()
{
  int32_t x, y;
  bool hay = lcd.getTouch(&x, &y);

  toqueNuevo  = (hay && !toqueActivo);   // flanco de bajada
  toqueActivo = hay;
  if (hay) { toqueX = x; toqueY = y; }
}

// Notificaciones de ejemplo mientras conectamos la PC
static void notificacionesDemo()
{
  notifAgregar("Tesla Lab",  "Build del firmware terminado", "09:41", NOTIF_INFO);
  notifAgregar("Pablo",      "Decirle gay a daniel",        "10:05", NOTIF_MENSAJE);
  notifAgregar("Bateria",    "Fuente externa desconectada",  "10:12", NOTIF_ALERTA);
  notifAgregar("Calendario", "Reunion con Dr. Rodas 13:00",  "10:30", NOTIF_INFO);
}

// ---------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[purple_deck] arrancando");

  displayIniciar();
  temaUsar(0);                 // 0 = "Afton" (morado + dorado)

  tiempoIniciar();
  notificacionesDemo();

  s_ultimaActividad = millis();
  entrarEn(pantallaActual);    // arranca en el menu principal
}

void loop()
{
  tiempoLoop();
  leerToque();

  // Mantener el dedo apoyado tambien cuenta como actividad
  if (toqueActivo) s_ultimaActividad = millis();

  switch (pantallaActual) {
    case SCREEN_MENU:       screenMenuLoop();       break;
    case SCREEN_MACROS:     screenMacrosLoop();     break;
    case SCREEN_SOLIDWORKS: screenSolidworksLoop(); break;
    case SCREEN_LABVIEW:    screenLabviewLoop();    break;
    case SCREEN_GIT:        screenGitLoop();        break;
    case SCREEN_AJUSTES:    screenAjustesLoop();    break;
    case SCREEN_ARDUINO:    screenArduinoLoop();    break;
    case SCREEN_IDLE:       screenIdleLoop();       break;
  }

  if (pantallaActual != SCREEN_IDLE &&
      (millis() - s_ultimaActividad > idleTimeoutMs)) {
    s_previaIdle = pantallaActual;
    irAPantalla(SCREEN_IDLE);
  }

  delay(10);
}
