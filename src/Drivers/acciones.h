#ifndef PURPLE_DECK_DRIVERS_ACCIONES_H_
#define PURPLE_DECK_DRIVERS_ACCIONES_H_
// ---------------------------------------------------------------------------
//  Drivers/acciones.h - todo lo que un boton puede "hacer"
//
//  Cada boton de un submenu apunta a un Accion. Las de Ajustes se ejecutan en
//  el propio deck; las demas (macros, SolidWorks, LabVIEW, Git, Arduino) son
//  atajos para la PC y por ahora solo se registran por Serial. Cuando exista
//  el enlace con la PC, basta con enviar alli el id en accionEjecutar().
// ---------------------------------------------------------------------------
#include <Arduino.h>

enum Accion : uint8_t {
  ACC_NINGUNA = 0,

  // Macros basicas
  ACC_VOL_UP, ACC_VOL_DOWN, ACC_MUTE,
  ACC_PLAY_PAUSA, ACC_SIGUIENTE, ACC_ANTERIOR,
  ACC_CAPTURA, ACC_ESCRITORIO, ACC_BLOQUEAR_PC,

  // SolidWorks
  ACC_SW_RECONSTRUIR, ACC_SW_AJUSTAR, ACC_SW_ISOMETRICA,
  ACC_SW_FRONTAL, ACC_SW_SUPERIOR, ACC_SW_NORMAL_A,

  // LabVIEW
  ACC_LV_EJECUTAR, ACC_LV_DETENER, ACC_LV_PANEL_DIAGRAMA,
  ACC_LV_AYUDA, ACC_LV_CABLES_ROTOS, ACC_LV_ORDENAR,

  // Git / VS Code
  ACC_GIT_STATUS, ACC_GIT_PULL, ACC_GIT_PUSH, ACC_GIT_COMMIT, ACC_GIT_ADD_ALL,
  ACC_VS_TERMINAL, ACC_VS_PALETA, ACC_VS_BUSCAR, ACC_VS_GUARDAR_TODO,

  // Arduino IDE
  ACC_ARD_VERIFICAR, ACC_ARD_SUBIR, ACC_ARD_MONITOR,
  ACC_ARD_PLOTTER, ACC_ARD_GUARDAR, ACC_ARD_LIBRERIAS,

  // Ajustes (se ejecutan en el deck)
  ACC_AJ_BRILLO_MENOS, ACC_AJ_BRILLO_MAS, ACC_AJ_REPOSO,
  ACC_AJ_RELOJ, ACC_AJ_TEMA, ACC_AJ_REINICIAR
};

// Ejecuta la accion. Devuelve true si cambio algo que obliga a redibujar la
// pantalla completa (por ejemplo, el tema).
bool accionEjecutar(Accion a);

#endif  // PURPLE_DECK_DRIVERS_ACCIONES_H_
