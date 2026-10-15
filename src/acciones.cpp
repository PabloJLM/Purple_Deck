#include "Drivers/acciones.h"
#include "Drivers/display.h"
#include "Estaticos/temas.h"
#include "Pantallas/screen_idle.h"
#include "config.h"

// ---------------------------------------------------------------------------
//  Tabla de acciones externas: a que atajo (o texto) corresponde cada una.
//  Sirve hoy para el log por Serial y mañana como contrato con la PC.
//  Los atajos son los de fabrica de cada programa: verificar contra la
//  configuracion real de cada uno.
// ---------------------------------------------------------------------------
struct AccionDef {
  Accion      id;
  const char *grupo;
  const char *nombre;
  const char *atajo;
};

static const AccionDef TABLA[] = {
  // Macros
  { ACC_VOL_UP,            "Macros",     "Vol +",            "Volume Up" },
  { ACC_VOL_DOWN,          "Macros",     "Vol -",            "Volume Down" },
  { ACC_MUTE,              "Macros",     "Mute",             "Volume Mute" },
  { ACC_PLAY_PAUSA,        "Macros",     "Play/Pausa",       "Media Play/Pause" },
  { ACC_SIGUIENTE,         "Macros",     "Siguiente",        "Media Next" },
  { ACC_ANTERIOR,          "Macros",     "Anterior",         "Media Previous" },
  { ACC_CAPTURA,           "Macros",     "Captura",          "Win+Shift+S" },
  { ACC_ESCRITORIO,        "Macros",     "Escritorio",       "Win+D" },
  { ACC_BLOQUEAR_PC,       "Macros",     "Bloquear PC",      "Win+L" },

  // SolidWorks
  { ACC_SW_RECONSTRUIR,    "SolidWorks", "Reconstruir",      "Ctrl+B" },
  { ACC_SW_AJUSTAR,        "SolidWorks", "Ajustar vista",    "F" },
  { ACC_SW_ISOMETRICA,     "SolidWorks", "Isometrica",       "Ctrl+7" },
  { ACC_SW_FRONTAL,        "SolidWorks", "Frontal",          "Ctrl+1" },
  { ACC_SW_SUPERIOR,       "SolidWorks", "Superior",         "Ctrl+5" },
  { ACC_SW_NORMAL_A,       "SolidWorks", "Normal a",         "Ctrl+8" },

  // LabVIEW
  { ACC_LV_EJECUTAR,       "LabVIEW",    "Ejecutar",         "Ctrl+R" },
  { ACC_LV_DETENER,        "LabVIEW",    "Detener",          "Ctrl+." },
  { ACC_LV_PANEL_DIAGRAMA, "LabVIEW",    "Panel/Diagrama",   "Ctrl+E" },
  { ACC_LV_AYUDA,          "LabVIEW",    "Ayuda contextual", "Ctrl+H" },
  { ACC_LV_CABLES_ROTOS,   "LabVIEW",    "Cables rotos",     "Ctrl+B" },
  { ACC_LV_ORDENAR,        "LabVIEW",    "Ordenar diagrama", "Ctrl+U" },

  // Git / VS Code (los de git son texto que se escribiria en la terminal)
  { ACC_GIT_STATUS,        "Git",        "Status",           "texto: git status" },
  { ACC_GIT_PULL,          "Git",        "Pull",             "texto: git pull" },
  { ACC_GIT_PUSH,          "Git",        "Push",             "texto: git push" },
  { ACC_GIT_COMMIT,        "Git",        "Commit",           "texto: git commit" },
  { ACC_GIT_ADD_ALL,       "Git",        "Add all",          "texto: git add ." },
  { ACC_VS_TERMINAL,       "VS Code",    "Terminal",         "Ctrl+`" },
  { ACC_VS_PALETA,         "VS Code",    "Paleta",           "Ctrl+Shift+P" },
  { ACC_VS_BUSCAR,         "VS Code",    "Buscar archivo",   "Ctrl+P" },
  { ACC_VS_GUARDAR_TODO,   "VS Code",    "Guardar todo",     "Ctrl+K S" },

  // Arduino IDE 2.x
  { ACC_ARD_VERIFICAR,     "Arduino",    "Verificar",        "Ctrl+R" },
  { ACC_ARD_SUBIR,         "Arduino",    "Subir",            "Ctrl+U" },
  { ACC_ARD_MONITOR,       "Arduino",    "Monitor serie",    "Ctrl+Shift+M" },
  { ACC_ARD_PLOTTER,       "Arduino",    "Plotter serie",    "Ctrl+Shift+L" },
  { ACC_ARD_GUARDAR,       "Arduino",    "Guardar",          "Ctrl+S" },
  { ACC_ARD_LIBRERIAS,     "Arduino",    "Librerias",        "Ctrl+Shift+I" },
};

static const AccionDef *buscar(Accion a)
{
  for (uint8_t i = 0; i < sizeof(TABLA) / sizeof(TABLA[0]); i++)
    if (TABLA[i].id == a) return &TABLA[i];
  return nullptr;
}

// ---------------------------------------------------------------------------
bool accionEjecutar(Accion a)
{
  switch (a) {

    // ---- Ajustes: se resuelven aqui mismo ----
    case ACC_AJ_BRILLO_MENOS: {
      int v = (int)brilloNormal - 25;
      brilloNormal = (v < 25) ? 25 : v;
      lcd.setBrightness(brilloNormal);
      Serial.printf("[ajustes] brillo %u\n", brilloNormal);
      return false;
    }
    case ACC_AJ_BRILLO_MAS: {
      int v = (int)brilloNormal + 25;
      brilloNormal = (v > 255) ? 255 : v;
      lcd.setBrightness(brilloNormal);
      Serial.printf("[ajustes] brillo %u\n", brilloNormal);
      return false;
    }
    case ACC_AJ_REPOSO: {
      static const uint32_t PASOS[] = { 10000UL, 20000UL, 30000UL, 60000UL };
      uint32_t siguiente = PASOS[0];                 // si ya es el mayor, da la vuelta
      for (uint8_t i = 0; i < sizeof(PASOS) / sizeof(PASOS[0]); i++)
        if (PASOS[i] > idleTimeoutMs) { siguiente = PASOS[i]; break; }
      idleTimeoutMs = siguiente;
      Serial.printf("[ajustes] reposo %lu s\n", (unsigned long)(idleTimeoutMs / 1000));
      return false;
    }
    case ACC_AJ_RELOJ:
      relojAlternarModo();
      Serial.printf("[ajustes] reloj %s\n",
                    relojModo() == RELOJ_ANALOGICO ? "analogico" : "digital");
      return false;

    case ACC_AJ_TEMA:
      temaUsar((temaIndice() + 1) % NUM_TEMAS);
      Serial.printf("[ajustes] tema %s\n", T().nombre);
      return true;                                   // cambian todos los colores

    case ACC_AJ_REINICIAR:
      Serial.println("[ajustes] reiniciando");
      delay(100);
      ESP.restart();
      return false;

    default:
      break;
  }

  // ---- Acciones para la PC: por ahora solo se registran ----
  const AccionDef *d = buscar(a);
  if (d) {
    Serial.printf("[accion] %s / %s -> %s  (sin enlace con la PC todavia)\n",
                  d->grupo, d->nombre, d->atajo);
  } else {
    Serial.printf("[accion] id %u sin definir\n", (unsigned)a);
  }
  return false;
}
