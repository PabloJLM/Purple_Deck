#include "config.h"
#include "Drivers/display.h"
#include "Drivers/tiempo.h"
#include "Drivers/notificaciones.h"
#include "Drivers/audio.h"
#include "Drivers/temporizadores.h"
#include "Estaticos/temas.h"
#include "Pantallas/screen_menu.h"
#include "Pantallas/screen_macros.h"
#include "Pantallas/screen_solidworks.h"
#include "Pantallas/screen_sw_rotar.h"
#include "Pantallas/screen_labview.h"
#include "Pantallas/screen_git.h"
#include "Pantallas/screen_blender.h"
#include "Pantallas/screen_ajustes.h"
#include "Pantallas/screen_slider.h"
#include "Pantallas/screen_temas.h"
#include "Pantallas/screen_pomodoro.h"
#include "Pantallas/screen_reloj.h"
#include "Pantallas/screen_idle.h"
#include "Pantallas/screen_slides.h"

Screen   pantallaActual = SCREEN_MENU;
bool     toqueNuevo  = false;
bool     toqueActivo = false;
bool     toqueSuelto = false;
bool     toquePropio = false;
int32_t  toqueX = 0, toqueY = 0;
int32_t  toqueX0 = 0, toqueY0 = 0;
uint32_t idleTimeoutMs = IDLE_TIMEOUT_MS;
uint8_t  brilloNormal  = BRILLO_NORMAL;
uint8_t  modoBloqueo   = BLOQUEO_RELOJ;
uint8_t  volumen       = VOLUMEN_INICIAL;

static uint32_t s_ultimaActividad = 0;
static Screen   s_previaIdle      = SCREEN_MENU;
static int8_t   s_bloqueoPedido   = -1;

static void salirDe(Screen s)
{
  switch (s) {
    case SCREEN_MENU:       screenMenuSalir();       break;
    case SCREEN_MACROS:     screenMacrosSalir();     break;
    case SCREEN_SOLIDWORKS: screenSolidworksSalir(); break;
    case SCREEN_SW_ROTAR:   screenSwRotarSalir();    break;
    case SCREEN_LABVIEW:    screenLabviewSalir();    break;
    case SCREEN_GIT:        screenGitSalir();        break;
    case SCREEN_BLENDER:    screenBlenderSalir();    break;
    case SCREEN_AJUSTES:    screenAjustesSalir();    break;
    case SCREEN_BRILLO:     screenBrilloSalir();     break;
    case SCREEN_REPOSO:     screenReposoSalir();     break;
    case SCREEN_VOLUMEN:    screenVolumenSalir();    break;
    case SCREEN_TEMAS:      screenTemasSalir();      break;
    case SCREEN_POMODORO:   screenPomodoroSalir();   break;
    case SCREEN_RELOJ:      screenRelojSalir();      break;
    case SCREEN_IDLE:       screenIdleSalir();       break;
    case SCREEN_SLIDES:     screenSlidesSalir();     break;
  }
}

static void entrarEn(Screen s)
{
  switch (s) {
    case SCREEN_MENU:       screenMenuEntrar();       break;
    case SCREEN_MACROS:     screenMacrosEntrar();     break;
    case SCREEN_SOLIDWORKS: screenSolidworksEntrar(); break;
    case SCREEN_SW_ROTAR:   screenSwRotarEntrar();    break;
    case SCREEN_LABVIEW:    screenLabviewEntrar();    break;
    case SCREEN_GIT:        screenGitEntrar();        break;
    case SCREEN_BLENDER:    screenBlenderEntrar();    break;
    case SCREEN_AJUSTES:    screenAjustesEntrar();    break;
    case SCREEN_BRILLO:     screenBrilloEntrar();     break;
    case SCREEN_REPOSO:     screenReposoEntrar();     break;
    case SCREEN_VOLUMEN:    screenVolumenEntrar();    break;
    case SCREEN_TEMAS:      screenTemasEntrar();      break;
    case SCREEN_POMODORO:   screenPomodoroEntrar();   break;
    case SCREEN_RELOJ:      screenRelojEntrar();      break;
    case SCREEN_IDLE:       screenIdleEntrar();       break;
    case SCREEN_SLIDES:     screenSlidesEntrar();     break;
  }
}

static void loopDe(Screen s)
{
  switch (s) {
    case SCREEN_MENU:       screenMenuLoop();       break;
    case SCREEN_MACROS:     screenMacrosLoop();     break;
    case SCREEN_SOLIDWORKS: screenSolidworksLoop(); break;
    case SCREEN_SW_ROTAR:   screenSwRotarLoop();    break;
    case SCREEN_LABVIEW:    screenLabviewLoop();    break;
    case SCREEN_GIT:        screenGitLoop();        break;
    case SCREEN_BLENDER:    screenBlenderLoop();    break;
    case SCREEN_AJUSTES:    screenAjustesLoop();    break;
    case SCREEN_BRILLO:     screenBrilloLoop();     break;
    case SCREEN_REPOSO:     screenReposoLoop();     break;
    case SCREEN_VOLUMEN:    screenVolumenLoop();    break;
    case SCREEN_TEMAS:      screenTemasLoop();      break;
    case SCREEN_POMODORO:   screenPomodoroLoop();   break;
    case SCREEN_RELOJ:      screenRelojLoop();      break;
    case SCREEN_IDLE:       screenIdleLoop();       break;
    case SCREEN_SLIDES:     screenSlidesLoop();     break;
  }
}

void irAPantalla(Screen s)
{
  if (s == pantallaActual) return;

  salirDe(pantallaActual);

  pantallaActual    = s;
  toqueNuevo        = false;
  toquePropio       = false;
  s_ultimaActividad = millis();

  entrarEn(pantallaActual);
}

void salirDeIdle()
{
  irAPantalla(s_previaIdle);
}

static void leerToque()
{
  int32_t x, y;
  const bool hay = lcd.getTouch(&x, &y);

  toqueNuevo  = hay && !toqueActivo;
  toqueSuelto = !hay && toqueActivo && toquePropio;
  toqueActivo = hay;
  if (hay) {
    toqueX = x; toqueY = y;
    if (toqueNuevo) { toqueX0 = x; toqueY0 = y; toquePropio = true; }
  }
}

void pedirBloqueo(uint8_t modo)
{
  s_bloqueoPedido = (int8_t)modo;
}

static bool bloqueado()
{
  return pantallaActual == SCREEN_IDLE || pantallaActual == SCREEN_SLIDES;
}

static bool mantenerDespierto()
{
  return (pantallaActual == SCREEN_POMODORO && pomoCorriendo()) ||
         (pantallaActual == SCREEN_RELOJ && cronoCorriendo());
}

// Notificaciones de ejemplo mientras conectamos la PC
static void notificacionesDemo()
{
  notifAgregar("Tesla Lab",  "Build del firmware terminado", "09:41", NOTIF_ALERTA);
  notifAgregar("Pablo",      "Decirle gay a daniel",        "10:05", NOTIF_MENSAJE);
  notifAgregar("Bateria",    "aaaaaaaa",  "10:12", NOTIF_ALERTA);
  notifAgregar("Calendario", "Reunion con Dr. Rodas 13:00",  "10:30", NOTIF_INFO);
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[purple_deck] arrancando");

  displayIniciar();
  temaUsar(0);
  slidesIniciar();

  tiempoIniciar();
  temporizadoresIniciar();
  notificacionesDemo();
  audioIniciar();

  s_ultimaActividad = millis();
  entrarEn(pantallaActual);
}

void loop()
{
  tiempoLoop();
  leerToque();

  if (toqueNuevo) alarmaSilenciar();
  if (temporizadoresLoop() && bloqueado()) salirDeIdle();

  if (toqueActivo) s_ultimaActividad = millis();

  loopDe(pantallaActual);

  if (s_bloqueoPedido >= 0) {
    const uint8_t modo = (uint8_t)s_bloqueoPedido;
    s_bloqueoPedido = -1;
    if (!bloqueado()) {
      s_previaIdle = pantallaActual;
      if (modo == BLOQUEO_SLIDES) slidesEscanear();
      irAPantalla(modo == BLOQUEO_SLIDES ? SCREEN_SLIDES : SCREEN_IDLE);
    }
  }

  if (!bloqueado() && !mantenerDespierto() && millis() - s_ultimaActividad > idleTimeoutMs) {
    s_previaIdle = pantallaActual;
    const bool fotos = (modoBloqueo == BLOQUEO_SLIDES) && slidesCantidad();
    irAPantalla(fotos ? SCREEN_SLIDES : SCREEN_IDLE);
  }

  delay(10);
}
