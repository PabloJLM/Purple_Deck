#include "Drivers/temporizadores.h"
#include "Drivers/tiempo.h"
#include "Drivers/notificaciones.h"
#include "Drivers/audio.h"
#include <Preferences.h>

#define ALARMA_SUENA_MS  15000UL
#define ALARMA_REPETIR_MS 2500UL
#define VUELTAS_MAX 3

static uint8_t  s_durMin[3] = { POMO_FOCO_MIN, POMO_CORTO_MIN, POMO_LARGO_MIN };
static PomoFase s_fase      = POMO_FOCO;
static uint8_t  s_ciclo     = 1;
static bool     s_pomoCorre = false;
static uint32_t s_pomoFin   = 0;
static uint32_t s_pomoResto = POMO_FOCO_MIN * 60000UL;

static bool     s_cronoCorre  = false;
static uint32_t s_cronoBase   = 0;
static uint32_t s_cronoInicio = 0;
static uint32_t s_vueltas[VUELTAS_MAX];
static uint8_t  s_nVueltas = 0;

static Alarma   s_alarmas[NUM_ALARMAS] = {
  { 7, 0, false }, { 12, 0, false }, { 18, 0, false }, { 22, 0, false }
};
static int32_t  s_clave[NUM_ALARMAS];
static uint32_t s_ultimaRevision = 0;
static bool     s_suena      = false;
static uint32_t s_finSuena   = 0;
static uint32_t s_proxSonido = 0;

static void horaTexto(char *b, size_t n)
{
  struct tm t;
  if (tiempoObtener(t)) snprintf(b, n, "%02d:%02d", t.tm_hour, t.tm_min);
  else                  snprintf(b, n, "--:--");
}

static uint32_t restanteMs()
{
  if (!s_pomoCorre) return s_pomoResto;
  const int32_t r = (int32_t)(s_pomoFin - millis());
  return r > 0 ? (uint32_t)r : 0;
}

static void cargarFase(PomoFase f)
{
  s_fase      = f;
  s_pomoResto = s_durMin[f] * 60000UL;
  s_pomoCorre = false;
}

static void arrancar()
{
  s_pomoFin   = millis() + s_pomoResto;
  s_pomoCorre = true;
}

static void siguienteFase()
{
  if (s_fase == POMO_FOCO) {
    cargarFase(s_ciclo >= POMO_CICLOS ? POMO_LARGO : POMO_CORTO);
  } else {
    s_ciclo = (s_fase == POMO_LARGO) ? 1 : s_ciclo + 1;
    cargarFase(POMO_FOCO);
  }
}

static void terminarFase()
{
  char hora[8], texto[NOTIF_TEXTO_LEN];
  horaTexto(hora, sizeof(hora));

  const bool eraFoco = (s_fase == POMO_FOCO);
  siguienteFase();

  if (eraFoco) {
    snprintf(texto, sizeof(texto), "%s de %u min",
             s_fase == POMO_LARGO ? "Descanso largo" : "Descanso", s_durMin[s_fase]);
    notifAgregar("Pomodoro", texto, hora, NOTIF_ALERTA);
    arrancar();
  } else {
    snprintf(texto, sizeof(texto), "A trabajar: foco de %u min", s_durMin[POMO_FOCO]);
    notifAgregar("Pomodoro", texto, hora, NOTIF_INFO);
  }
}

void pomoIniciarPausar()
{
  if (s_pomoCorre) { s_pomoResto = restanteMs(); s_pomoCorre = false; }
  else             { arrancar(); }
}

void pomoReiniciar()
{
  s_ciclo = 1;
  cargarFase(POMO_FOCO);
}

void pomoSaltar()
{
  siguienteFase();
}

bool pomoCorriendo() { return s_pomoCorre; }

bool pomoIniciado()
{
  return s_pomoCorre || s_pomoResto != s_durMin[s_fase] * 60000UL;
}

void pomoAjustar(int8_t deltaMin)
{
  if (pomoIniciado()) return;
  s_durMin[s_fase] = (uint8_t)constrain((int)s_durMin[s_fase] + deltaMin, 1, 90);
  s_pomoResto = s_durMin[s_fase] * 60000UL;
}

PomoFase pomoFase() { return s_fase; }
uint8_t  pomoCiclo() { return s_ciclo; }
uint8_t  pomoDuracionMin() { return s_durMin[s_fase]; }
uint32_t pomoRestanteS() { return (restanteMs() + 999) / 1000; }
uint32_t pomoTotalS() { return s_durMin[s_fase] * 60UL; }

uint32_t cronoMs()
{
  return s_cronoBase + (s_cronoCorre ? millis() - s_cronoInicio : 0);
}

void cronoIniciarPausar()
{
  if (s_cronoCorre) { s_cronoBase = cronoMs(); s_cronoCorre = false; }
  else              { s_cronoInicio = millis(); s_cronoCorre = true; }
}

void cronoReiniciar()
{
  s_cronoCorre = false;
  s_cronoBase  = 0;
  s_nVueltas   = 0;
}

void cronoVuelta()
{
  if (!s_cronoCorre) return;
  for (uint8_t i = VUELTAS_MAX - 1; i > 0; i--) s_vueltas[i] = s_vueltas[i - 1];
  s_vueltas[0] = cronoMs();
  s_nVueltas++;
}

bool     cronoCorriendo() { return s_cronoCorre; }
uint8_t  cronoVueltas() { return s_nVueltas; }
uint32_t cronoVueltaMs(uint8_t i) { return s_vueltas[i % VUELTAS_MAX]; }

Alarma &alarma(uint8_t i) { return s_alarmas[i % NUM_ALARMAS]; }

void alarmasGuardar()
{
  Preferences p;
  if (!p.begin("deck", false)) return;
  p.putBytes("alarmas", s_alarmas, sizeof(s_alarmas));
  p.end();
}

void alarmaSilenciar() { s_suena = false; }

float tempChip() { return temperatureRead(); }

void temporizadoresIniciar()
{
  Preferences p;
  if (p.begin("deck", true)) {
    if (p.getBytesLength("alarmas") == sizeof(s_alarmas)) p.getBytes("alarmas", s_alarmas, sizeof(s_alarmas));
    p.end();
  }
  for (uint8_t i = 0; i < NUM_ALARMAS; i++) {
    s_alarmas[i].hora   %= 24;
    s_alarmas[i].minuto %= 60;
    s_clave[i] = -1;
  }
}

static bool revisarAlarmas()
{
  struct tm t;
  if (!tiempoObtener(t)) return false;

  bool sono = false;
  for (uint8_t i = 0; i < NUM_ALARMAS; i++) {
    const Alarma &a = s_alarmas[i];
    const int32_t clave = t.tm_yday * 1440 + t.tm_hour * 60 + t.tm_min;
    if (!a.activa || a.hora != t.tm_hour || a.minuto != t.tm_min || s_clave[i] == clave) continue;

    s_clave[i] = clave;
    char hora[8], texto[NOTIF_TEXTO_LEN];
    snprintf(hora, sizeof(hora), "%02d:%02d", a.hora, a.minuto);
    snprintf(texto, sizeof(texto), "Alarma de las %s", hora);
    notifAgregar("Alarma", texto, hora, NOTIF_ALERTA);
    sono = true;
  }
  return sono;
}

bool temporizadoresLoop()
{
  const uint32_t ahora = millis();
  bool despertar = false;

  if (s_pomoCorre && (int32_t)(s_pomoFin - ahora) <= 0) {
    terminarFase();
    despertar = true;
  }

  if (ahora - s_ultimaRevision >= 1000) {
    s_ultimaRevision = ahora;
    if (revisarAlarmas()) {
      s_suena      = true;
      s_finSuena   = ahora + ALARMA_SUENA_MS;
      s_proxSonido = ahora + ALARMA_REPETIR_MS;
      despertar    = true;
    }
  }

  if (s_suena) {
    if ((int32_t)(s_finSuena - ahora) <= 0) {
      s_suena = false;
    } else if ((int32_t)(s_proxSonido - ahora) <= 0) {
      s_proxSonido = ahora + ALARMA_REPETIR_MS;
      audioReproducir(SONIDO_ALERTA);
    }
  }

  return despertar;
}
