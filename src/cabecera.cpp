#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "config.h"

#define BTN_X GAP
#define BTN_Y GAP
#define BTN_W 140
#define BTN_H (CAB_ALTO - 2 * GAP)

void cabeceraDibujar(const char *titulo, const char *atras)
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  g->fillRect(0, 0, ANCHO, CAB_ALTO, t.fondo);
  g->setTextSize(1.0f);
  g->fillRoundRect(BTN_X, BTN_Y, BTN_W, BTN_H, RAD, t.superficie);
  g->drawRoundRect(BTN_X, BTN_Y, BTN_W, BTN_H, RAD, t.borde);
  g->setFont(&fonts::Font4);
  g->setTextDatum(middle_center);
  g->setTextColor(t.acento);
  g->drawString(atras, BTN_X + BTN_W / 2, BTN_Y + BTN_H / 2);
  g->setTextColor(t.texto);
  g->drawString(titulo, (BTN_X + BTN_W + ANCHO) / 2, CAB_ALTO / 2);
}

bool cabeceraAtras()
{
  return toqueNuevo && toqueX >= BTN_X && toqueX < BTN_X + BTN_W &&
         toqueY >= BTN_Y && toqueY < BTN_Y + BTN_H;
}
