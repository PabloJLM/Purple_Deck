#include "Drivers/acciones.h"
#include "Drivers/display.h"
#include "Drivers/teclado.h"
#include "Drivers/slots.h"
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
  { ACC_SW_ABRIR,          "SolidWorks", "Abrir",            "Ctrl+O" },
  { ACC_SW_NUEVO,          "SolidWorks", "Nuevo archivo",    "Ctrl+N" },
  { ACC_SW_ROT_IZQ,        "SolidWorks", "Rotar izq",        "Left" },
  { ACC_SW_ROT_DER,        "SolidWorks", "Rotar der",        "Right" },
  { ACC_SW_ROT_ARR,        "SolidWorks", "Rotar arriba",     "Up" },
  { ACC_SW_ROT_ABA,        "SolidWorks", "Rotar abajo",      "Down" },
  { ACC_SW_MOV_IZQ,        "SolidWorks", "Mover izq",        "Ctrl+Left" },
  { ACC_SW_MOV_DER,        "SolidWorks", "Mover der",        "Ctrl+Right" },
  { ACC_SW_MOV_ARR,        "SolidWorks", "Mover arriba",     "Ctrl+Up" },
  { ACC_SW_MOV_ABA,        "SolidWorks", "Mover abajo",      "Ctrl+Down" },

  { ACC_LV_EJECUTAR,       "LabVIEW",    "Ejecutar",         "Ctrl+R" },
  { ACC_LV_DETENER,        "LabVIEW",    "Detener",          "Ctrl+." },
  { ACC_LV_PANEL_DIAGRAMA, "LabVIEW",    "Panel/Diagrama",   "Ctrl+E" },
  { ACC_LV_AYUDA,          "LabVIEW",    "Ayuda contextual", "Ctrl+H" },
  { ACC_LV_CABLES_ROTOS,   "LabVIEW",    "Cables rotos",     "Ctrl+B" },
  { ACC_LV_ORDENAR,        "LabVIEW",    "Ordenar diagrama", "Ctrl+U" },

  { ACC_GIT_STATUS,        "Git",        "Status",           "texto: git status" },
  { ACC_GIT_PULL,          "Git",        "Pull",             "texto: git pull" },
  { ACC_GIT_PUSH,          "Git",        "Push",             "texto: git push" },
  { ACC_GIT_COMMIT,        "Git",        "Commit",           "texto: git commit -m " },
  { ACC_GIT_ADD_ALL,       "Git",        "Add all",          "texto: git add ." },
  { ACC_VS_TERMINAL,       "VS Code",    "Terminal",         "Ctrl+` (US) / Ctrl+N~ (ES)" },
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
  { ACC_CN_ENVIAR,         "Candle",     "Enviar G-code",    "F5" },
  { ACC_CN_PAUSA,          "Candle",     "Pausa",            "F6" },
  { ACC_CN_ABORTAR,        "Candle",     "Abortar",          "F7" },
  { ACC_CN_DESBLOQUEAR,    "Candle",     "Desbloquear",      "F9" },
  { ACC_CN_RESET,          "Candle",     "Reset",            "F11" },
  { ACC_CN_ZERO_XY,        "Candle",     "Zero XY",          "F8 (comando usuario 1)" },
  { ACC_CN_ZERO_Z,         "Candle",     "Zero Z",           "Ctrl+F8 (comando usuario 2)" },
  { ACC_CN_IR_XY0,         "Candle",     "Ir a X0 Y0",       "Ctrl+F7 (comando usuario 4)" },
  { ACC_CN_PROBE_Z,        "Candle",     "Probe Z",          "Ctrl+F9 (comando usuario 3)" },
  { ACC_CN_HEIGHTMAP,      "Candle",     "Crear heightmap",  "Ctrl+F6" },
  { ACC_CN_HUSILLO,        "Candle",     "Husillo on/off",   "Num 0" },
  { ACC_CN_X_MENOS,        "Candle",     "X-",               "Num 4" },
  { ACC_CN_X_MAS,          "Candle",     "X+",               "Num 6" },
  { ACC_CN_Y_MENOS,        "Candle",     "Y-",               "Num 2" },
  { ACC_CN_Y_MAS,          "Candle",     "Y+",               "Num 8" },
  { ACC_CN_Z_MENOS,        "Candle",     "Z-",               "Num 3" },
  { ACC_CN_Z_MAS,          "Candle",     "Z+",               "Num 9" },
  { ACC_CN_PARAR,          "Candle",     "Parar jog",        "Num 5" },
};

// --- catalogo para los espacios editables --------------------------------------
static void hacerClave(const AccionDef &d, char *k, size_t cap)
{
  size_t o = 0;
  auto agregar = [&](char c) { if (o + 1 < cap) k[o++] = c; };
  auto frase = [&](const char *t) {
    bool sep = false;
    for (; *t; t++) {
      char c = *t;
      if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
      if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
        if (sep) agregar('_');
        agregar(c);
        sep = false;
      } else if (c == '+') { if (o) agregar('_'); agregar('m'); agregar('a'); agregar('s'); sep = false; }
      else if (c == '-')   { if (o) agregar('_'); for (const char *m = "menos"; *m; m++) agregar(*m); sep = false; }
      else if (o)          { sep = true; }
    }
  };
  frase(d.grupo);
  agregar('.');
  frase(d.nombre);
  k[o] = '\0';
}

uint8_t accionesCantidad() { return sizeof(TABLA) / sizeof(TABLA[0]); }

bool accionInfo(uint8_t i, char *clave, size_t cap,
                const char **grupo, const char **nombre, const char **atajo)
{
  if (i >= accionesCantidad()) return false;
  hacerClave(TABLA[i], clave, cap);
  *grupo = TABLA[i].grupo;
  *nombre = TABLA[i].nombre;
  *atajo = TABLA[i].atajo;
  return true;
}

bool accionPorClave(const char *clave, Accion *out)
{
  char k[40];
  for (uint8_t i = 0; i < accionesCantidad(); i++) {
    hacerClave(TABLA[i], k, sizeof(k));
    if (strcmp(k, clave) == 0) { *out = TABLA[i].id; return true; }
  }
  return false;
}

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

    // SolidWorks
    case ACC_SW_AJUSTAR:   tecladoAtajo({ 'f' });              return false;
    case ACC_SW_FRONTAL:   tecladoAtajo({ TECLA_CTRL, '1' });  return false;
    case ACC_SW_SUPERIOR:  tecladoAtajo({ TECLA_CTRL, '5' });  return false;
    case ACC_SW_LATERAL:   tecladoAtajo({ TECLA_CTRL, '4' });  return false;
    case ACC_SW_ABRIR:     tecladoAtajo({ TECLA_CTRL, 'o' });  return false;
    case ACC_SW_NUEVO:     tecladoAtajo({ TECLA_CTRL, 'n' });  return false;
    case ACC_SW_ROT_IZQ:   tecladoAtajo({ TECLA_IZQ });        return false;
    case ACC_SW_ROT_DER:   tecladoAtajo({ TECLA_DER });        return false;
    case ACC_SW_ROT_ARR:   tecladoAtajo({ TECLA_ARRIBA });     return false;
    case ACC_SW_ROT_ABA:   tecladoAtajo({ TECLA_ABAJO });      return false;
    case ACC_SW_MOV_IZQ:   tecladoAtajo({ TECLA_CTRL, TECLA_IZQ });    return false;
    case ACC_SW_MOV_DER:   tecladoAtajo({ TECLA_CTRL, TECLA_DER });    return false;
    case ACC_SW_MOV_ARR:   tecladoAtajo({ TECLA_CTRL, TECLA_ARRIBA }); return false;
    case ACC_SW_MOV_ABA:   tecladoAtajo({ TECLA_CTRL, TECLA_ABAJO });  return false;

    // LabVIEW
    case ACC_LV_EJECUTAR:       tecladoAtajo({ TECLA_CTRL, 'r' }); return false;
    case ACC_LV_DETENER:        tecladoAtajo({ TECLA_CTRL, '.' }); return false;
    case ACC_LV_PANEL_DIAGRAMA: tecladoAtajo({ TECLA_CTRL, 'e' }); return false;
    case ACC_LV_AYUDA:          tecladoAtajo({ TECLA_CTRL, 'h' }); return false;
    case ACC_LV_CABLES_ROTOS:   tecladoAtajo({ TECLA_CTRL, 'b' }); return false;
    case ACC_LV_ORDENAR:        tecladoAtajo({ TECLA_CTRL, 'u' }); return false;

    // Git: se escribe el comando en la terminal que tenga el foco
    case ACC_GIT_STATUS:   tecladoEscribir("git status", true);     return false;
    case ACC_GIT_PULL:     tecladoEscribir("git pull", true);       return false;
    case ACC_GIT_PUSH:     tecladoEscribir("git push", true);       return false;
    case ACC_GIT_ADD_ALL:  tecladoEscribir("git add .", true);      return false;
    case ACC_GIT_COMMIT:   tecladoEscribir("git commit -m ", false); return false;  // falta el mensaje

    // VS Code
    case ACC_VS_TERMINAL:  // VS Code usa Ctrl+` en US y Ctrl+Ñ en distribuciones en espanol
      if (tecladoDistribucionActual() == DIST_US) tecladoAtajo({ TECLA_CTRL, '`' });
      else                                        tecladoAtajo({ TECLA_CTRL, TECLA_ENYE });
      return false;
    case ACC_VS_PALETA:        tecladoAtajo({ TECLA_CTRL, TECLA_SHIFT, 'p' });  return false;
    case ACC_VS_BUSCAR:        tecladoAtajo({ TECLA_CTRL, 'p' });               return false;
    case ACC_VS_GUARDAR_TODO:  tecladoAtajo({ TECLA_CTRL, 'k' }); tecladoAtajo({ 's' });             return false;
    case ACC_VS_ABRIR_CARPETA: tecladoAtajo({ TECLA_CTRL, 'k' }); tecladoAtajo({ TECLA_CTRL, 'o' }); return false;
    case ACC_VS_TERMINAL_EXT:  tecladoAtajo({ TECLA_CTRL, TECLA_SHIFT, 'c' });  return false;

    // Blender
    case ACC_BL_FRONTAL:   tecladoAtajo({ TECLA_NUM1 });               return false;
    case ACC_BL_LATERAL:   tecladoAtajo({ TECLA_NUM3 });               return false;
    case ACC_BL_SUPERIOR:  tecladoAtajo({ TECLA_NUM7 });               return false;
    case ACC_BL_ENCUADRAR: tecladoAtajo({ TECLA_HOME });               return false;
    case ACC_BL_EDICION:   tecladoAtajo({ TECLA_TAB });                return false;
    case ACC_BL_EXTRUIR:   tecladoAtajo({ 'e' });                      return false;
    case ACC_BL_AGREGAR:   tecladoAtajo({ TECLA_SHIFT, 'a' });         return false;
    case ACC_BL_RENDER:    tecladoAtajo({ TECLA_F12 });                return false;
    case ACC_BL_GUARDAR:   tecladoAtajo({ TECLA_CTRL, 's' });          return false;

    // Candle CNC. Candle no trae atajos para casi nada: hay que asignarlos
    // (ver README). Las F-keys no dependen de la distribucion del teclado.
    // Los de movimiento usan el teclado numerico, que Candle ya trae asignado
    // (y solo funcionan con Bloq Num activo y "Keyboard control" encendido).
    case ACC_CN_ENVIAR:        tecladoAtajo({ TECLA_F5 });               return false;
    case ACC_CN_PAUSA:         tecladoAtajo({ TECLA_F6 });               return false;
    case ACC_CN_ABORTAR:       tecladoAtajo({ TECLA_F7 });               return false;
    case ACC_CN_DESBLOQUEAR:   tecladoAtajo({ TECLA_F9 });               return false;
    case ACC_CN_RESET:         tecladoAtajo({ TECLA_F11 });              return false;
    case ACC_CN_ZERO_XY:       tecladoAtajo({ TECLA_F8 });               return false;
    case ACC_CN_ZERO_Z:        tecladoAtajo({ TECLA_CTRL, TECLA_F8 });   return false;
    case ACC_CN_IR_XY0:        tecladoAtajo({ TECLA_CTRL, TECLA_F7 });   return false;
    case ACC_CN_PROBE_Z:       tecladoAtajo({ TECLA_CTRL, TECLA_F9 });   return false;
    case ACC_CN_HEIGHTMAP:     tecladoAtajo({ TECLA_CTRL, TECLA_F6 });   return false;
    case ACC_CN_HUSILLO:       tecladoAtajo({ TECLA_NUM0 });             return false;
    case ACC_CN_X_MENOS:       tecladoAtajo({ TECLA_NUM4 });             return false;
    case ACC_CN_X_MAS:         tecladoAtajo({ TECLA_NUM6 });             return false;
    case ACC_CN_Y_MENOS:       tecladoAtajo({ TECLA_NUM2 });             return false;
    case ACC_CN_Y_MAS:         tecladoAtajo({ TECLA_NUM8 });             return false;
    case ACC_CN_Z_MENOS:       tecladoAtajo({ TECLA_NUM3 });             return false;
    case ACC_CN_Z_MAS:         tecladoAtajo({ TECLA_NUM9 });             return false;
    case ACC_CN_PARAR:         tecladoAtajo({ TECLA_NUM5 });             return false;

    case ACC_BLOQUEAR_YA:
      pedirBloqueo(BLOQUEO_RELOJ);
      return false;

    case ACC_FOTOS_YA:
      pedirBloqueo(BLOQUEO_SLIDES);
      return false;

    case ACC_SLOT_1:
    case ACC_SLOT_2:
    case ACC_SLOT_3:
      slotEjecutar((uint8_t)(a - ACC_SLOT_1));
      return false;

    case ACC_AJ_TECLADO:
      tecladoDistribucion((tecladoDistribucionActual() + 1) % NUM_DIST);
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
