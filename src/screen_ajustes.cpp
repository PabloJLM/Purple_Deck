#include "Pantallas/screen_ajustes.h"
#include "Pantallas/screen_idle.h"
#include "Pantallas/screen_slider.h"
#include "Pantallas/screen_slides.h"
#include "Estaticos/rejilla.h"
#include "Estaticos/temas.h"
#include "Drivers/acciones.h"
#include "config.h"
#include "secrets.h"

static const char *subReloj()
{
  return relojModo() == RELOJ_ANALOGICO ? "Analogico" : "Digital";
}

static const char *subTema()
{
  return T().nombre;
}

static const char *subRed()
{
  return WIFI_SSID;
}

static const char *subBloqueo()
{
  if (modoBloqueo == BLOQUEO_RELOJ) return "Reloj";
  if (!slidesCantidad()) return "Fotos: ninguna";
  static char b[24];
  snprintf(b, sizeof(b), "Fotos %s (%u)", slidesOrigen(), (unsigned)slidesCantidad());
  return b;
}

static const Item ITEMS[] = {
  IT_PANT_DIN("Brillo",  nullptr, SCREEN_BRILLO,  textoBrillo),
  IT_PANT_DIN("Reposo",  nullptr, SCREEN_REPOSO,  textoReposo),
  IT_PANT_DIN("Volumen", nullptr, SCREEN_VOLUMEN, textoVolumen),
  IT_ACC_DIN("Reloj",    nullptr, ACC_AJ_RELOJ,   subReloj),
  IT_PANT_DIN("Tema",    nullptr, SCREEN_TEMAS,   subTema),
  IT_ACC_DIN("Bloqueo",  nullptr, ACC_AJ_BLOQUEO, subBloqueo),
  IT_PANT_DIN("Credenciales", nullptr, SCREEN_CREDENCIALES, subRed),
  IT_PANT("Sistema", "Memoria y espacio", nullptr, SCREEN_SISTEMA),
  IT_PANT("Servidor FTP", "Copiar archivos por WiFi", nullptr, SCREEN_FTP),
  IT_ACC("Reiniciar", "Reinicia el deck", nullptr, ACC_AJ_REINICIAR),
};

static const RejillaCfg CFG = {
  ITEMS, sizeof(ITEMS) / sizeof(ITEMS[0]),
  3,
  true,
  SCREEN_MENU
};

void screenAjustesEntrar() { rejillaEntrar(CFG); }
void screenAjustesLoop()   { rejillaLoop();      }
void screenAjustesSalir()  { rejillaSalir();     }
