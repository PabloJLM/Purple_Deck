#ifndef PURPLE_DECK_DRIVERS_NOTIFICACIONES_H_
#define PURPLE_DECK_DRIVERS_NOTIFICACIONES_H_
// ---------------------------------------------------------------------------
//  Drivers/notificaciones.h - buffer circular de avisos
//
//  Por ahora se llena a mano (notifAgregar). Cuando conectemos la PC, el
//  parser solo tiene que llamar a notifAgregar() y la pantalla se entera sola
//  por notifVersion().
// ---------------------------------------------------------------------------
#include <Arduino.h>

#define NOTIF_MAX        8
#define NOTIF_TITULO_LEN 22
#define NOTIF_TEXTO_LEN  46

enum NotifTipo : uint8_t {
  NOTIF_INFO = 0,
  NOTIF_MENSAJE,
  NOTIF_ALERTA
};

struct Notificacion {
  char      titulo[NOTIF_TITULO_LEN];
  char      texto[NOTIF_TEXTO_LEN];
  char      hora[6];        // "14:32"
  NotifTipo tipo;
};

void   notifAgregar(const char *titulo, const char *texto,
                    const char *hora, NotifTipo tipo = NOTIF_INFO);
void   notifLimpiar();
uint8_t notifCantidad();                      // cuantas hay guardadas
const Notificacion &notifObtener(uint8_t i);  // 0 = la mas reciente
uint32_t notifVersion();                      // cambia con cada modificacion

#endif  // PURPLE_DECK_DRIVERS_NOTIFICACIONES_H_
