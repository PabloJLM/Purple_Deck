#include "Drivers/notificaciones.h"

static Notificacion s_lista[NOTIF_MAX];
static uint8_t      s_cantidad = 0;
static uint32_t     s_version  = 0;

static void copiar(char *destino, size_t largo, const char *origen)
{
  if (!origen) { destino[0] = '\0'; return; }
  strncpy(destino, origen, largo - 1);
  destino[largo - 1] = '\0';
}

void notifAgregar(const char *titulo, const char *texto,
                  const char *hora, NotifTipo tipo)
{
  // desplaza para que el indice 0 sea siempre la mas reciente
  uint8_t tope = (s_cantidad < NOTIF_MAX) ? s_cantidad : (NOTIF_MAX - 1);
  for (int8_t i = tope; i > 0; i--) s_lista[i] = s_lista[i - 1];

  copiar(s_lista[0].titulo, NOTIF_TITULO_LEN, titulo);
  copiar(s_lista[0].texto,  NOTIF_TEXTO_LEN,  texto);
  copiar(s_lista[0].hora,   sizeof(s_lista[0].hora), hora);
  s_lista[0].tipo = tipo;

  if (s_cantidad < NOTIF_MAX) s_cantidad++;
  s_version++;
}

void notifLimpiar()
{
  s_cantidad = 0;
  s_version++;
}

uint8_t notifCantidad() { return s_cantidad; }

const Notificacion &notifObtener(uint8_t i)
{
  if (i >= s_cantidad) i = 0;
  return s_lista[i];
}

uint32_t notifVersion() { return s_version; }
