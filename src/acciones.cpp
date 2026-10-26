#include "Drivers/acciones.h"
#include "Drivers/display.h"
#include "Drivers/teclado.h"
#include "Pantallas/screen_idle.h"
#include "Pantallas/screen_slides.h"
#include "config.h"

struct AccionDef {
  Accion      id;
  const char *grupo;
  const char *nombre;
  const char *atajo;
};

static const AccionDef TABLA[] = {
  { ACC_VOL_UP,            "Macros",     "Vol +",            "Volume Up" },
  { ACC_VOL_DOWN,          "Macros",     "Vol -",            "Volume Down" },
  { ACC_MUTE,              "Macros",     "Mute",             "Volume Mute" },
  { ACC_PLAY_PAUSA,        "Macros",     "Play/Pausa",       "Media Play/Pause" },
  { ACC_SIGUIENTE,         "Macros",     "Siguiente",        "Media Next" },
  { ACC_ANTERIOR,          "Macros",     "Anterior",         "Media Previous" },
  { ACC_CAPTURA,           "Macros",     "Captura",          "Win+Shift+S" },
  { ACC_ESCRITORIO,        "Macros",     "Escritorio",       "Win+D" },
  { ACC_BLOQUEAR_PC,       "Macros",     "Bloquear PC",      "Win+L" },
  { ACC_ABRIR_SPOTIFY,     "Macros",     "Spotify",          "abrir: spotify" },
  { ACC_ABRIR_NAVEGADOR,   "Macros",     "Navegador",        "abrir: navegador" },

  { ACC_SW_AJUSTAR,        "SolidWorks", "Ajustar vista",    "F" },
  { ACC_SW_FRONTAL,        "SolidWorks", "Frontal",          "Ctrl+1" },
  { ACC_SW_SUPERIOR,       "SolidWorks", "Superior",         "Ctrl+5" },
  { ACC_SW_LATERAL,        "SolidWorks", "Lateral",          "Ctrl+4" },
  { ACC_SW_EXTRUIR,        "SolidWorks", "Extruir",          "sin atajo de fabrica" },
  { ACC_SW_SKETCH,         "SolidWorks", "Sketch",           "sin atajo de fabrica" },
  { ACC_SW_ROT_IZQ,        "SolidWorks", "Rotar izq",        "Left" },
  { ACC_SW_ROT_DER,        "SolidWorks", "Rotar der",        "Right" },
  { ACC_SW_ROT_ARR,        "SolidWorks", "Rotar arriba",     "Up" },
  { ACC_SW_ROT_ABA,        "SolidWorks", "Rotar abajo",      "Down" },

  { ACC_LV_EJECUTAR,       "LabVIEW",    "Ejecutar",         "Ctrl+R" },
  { ACC_LV_DETENER,        "LabVIEW",    "Detener",          "Ctrl+." },
  { ACC_LV_PANEL_DIAGRAMA, "LabVIEW",    "Panel/Diagrama",   "Ctrl+E" },
  { ACC_LV_AYUDA,          "LabVIEW",    "Ayuda contextual", "Ctrl+H" },
  { ACC_LV_CABLES_ROTOS,   "LabVIEW",    "Cables rotos",     "Ctrl+B" },
  { ACC_LV_ORDENAR,        "LabVIEW",    "Ordenar diagrama", "Ctrl+U" },

  { ACC_GIT_STATUS,        "Git",        "Status",           "texto: git status" },
  { ACC_GIT_PULL,          "Git",        "Pull",             "texto: git pull" },
  { ACC_GIT_PUSH,          "Git",        "Push",             "texto: git push" },
  { ACC_GIT_COMMIT,        "Git",        "Commit",           "texto: git commit" },
  { ACC_GIT_ADD_ALL,       "Git",        "Add all",          "texto: git add ." },
  { ACC_VS_TERMINAL,       "VS Code",    "Terminal",         "Ctrl+`" },
  { ACC_VS_PALETA,         "VS Code",    "Paleta",           "Ctrl+Shift+P" },
  { ACC_VS_BUSCAR,         "VS Code",    "Buscar archivo",   "Ctrl+P" },
  { ACC_VS_GUARDAR_TODO,   "VS Code",    "Guardar todo",     "Ctrl+K S" },
  { ACC_VS_ABRIR_CARPETA,  "VS Code",    "Abrir carpeta",    "Ctrl+K Ctrl+O" },
  { ACC_VS_TERMINAL_EXT,   "VS Code",    "Terminal externa", "Ctrl+Shift+C" },

  { ACC_BL_FRONTAL,        "Blender",    "Frontal",          "Numpad 1" },
  { ACC_BL_LATERAL,        "Blender",    "Lateral",          "Numpad 3" },
  { ACC_BL_SUPERIOR,       "Blender",    "Superior",         "Numpad 7" },
  { ACC_BL_ENCUADRAR,      "Blender",    "Encuadrar todo",   "Home" },
  { ACC_BL_EDICION,        "Blender",    "Modo edicion",     "Tab" },
  { ACC_BL_EXTRUIR,        "Blender",    "Extruir",          "E" },
  { ACC_BL_AGREGAR,        "Blender",    "Agregar",          "Shift+A" },
  { ACC_BL_RENDER,         "Blender",    "Render",           "F12" },
  { ACC_BL_GUARDAR,        "Blender",    "Guardar",          "Ctrl+S" },
};

static const AccionDef *buscar(Accion a)
{
  for (uint8_t i = 0; i < sizeof(TABLA) / sizeof(TABLA[0]); i++)
    if (TABLA[i].id == a) return &TABLA[i];
  return nullptr;
}

bool accionEjecutar(Accion a)
{
  switch (a) {
    case ACC_AJ_RELOJ:
      relojAlternarModo();
      Serial.printf("[ajustes] reloj %s\n",
                    relojModo() == RELOJ_ANALOGICO ? "analogico" : "digital");
      return false;

    case ACC_AJ_BLOQUEO:
      modoBloqueo = (modoBloqueo == BLOQUEO_RELOJ) ? BLOQUEO_SLIDES : BLOQUEO_RELOJ;
      if (modoBloqueo == BLOQUEO_SLIDES) slidesEscanear();
      Serial.printf("[ajustes] bloqueo %s\n", modoBloqueo == BLOQUEO_RELOJ ? "reloj" : "slides");
      return false;

    case ACC_VOL_UP:       tecladoMedia(MEDIA_VOL_UP);    return false;
    case ACC_VOL_DOWN:     tecladoMedia(MEDIA_VOL_DOWN);  return false;
    case ACC_MUTE:         tecladoMedia(MEDIA_MUTE);      return false;
    case ACC_PLAY_PAUSA:   tecladoMedia(MEDIA_PLAY_PAUSA); return false;
    case ACC_SIGUIENTE:    tecladoMedia(MEDIA_SIGUIENTE); return false;
    case ACC_ANTERIOR:     tecladoMedia(MEDIA_ANTERIOR);  return false;
    case ACC_CAPTURA:      tecladoAtajo({ TECLA_WIN, TECLA_SHIFT, 's' }); return false;
    case ACC_ESCRITORIO:   tecladoAtajo({ TECLA_WIN, 'd' }); return false;
    case ACC_BLOQUEAR_PC:  tecladoAtajo({ TECLA_WIN, 'l' }); return false;
    case ACC_ABRIR_SPOTIFY:   tecladoAbrirApp(APP_SPOTIFY);   return false;
    case ACC_ABRIR_NAVEGADOR: tecladoAbrirApp(APP_NAVEGADOR); return false;

    case ACC_BLOQUEAR_YA:
      pedirBloqueo(BLOQUEO_RELOJ);
      return false;

    case ACC_FOTOS_YA:
      pedirBloqueo(BLOQUEO_SLIDES);
      return false;

    case ACC_AJ_REINICIAR:
      Serial.println("[ajustes] reiniciando");
      delay(100);
      ESP.restart();
      return false;

    default:
      break;
  }

  const AccionDef *d = buscar(a);
  if (d) Serial.printf("[accion] %s / %s -> %s  (sin enlace con la PC todavia)\n",
                       d->grupo, d->nombre, d->atajo);
  else   Serial.printf("[accion] id %u sin definir\n", (unsigned)a);
  return false;
}
