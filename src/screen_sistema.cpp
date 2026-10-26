#include "Pantallas/screen_sistema.h"
#include "Estaticos/cabecera.h"
#include "Estaticos/temas.h"
#include "Drivers/display.h"
#include "Pantallas/screen_slides.h"
#include "config.h"
#include <SD.h>

#define ZONA_Y     (CAB_ALTO + 6)
#define BARRA_H    14
#define BARRA_W    (ANCHO - 2 * GAP - 150)
#define FILA_ALTO  52

struct Fila { const char *etiqueta; uint64_t usado; uint64_t total; bool disponible; };

static void textoKB(uint64_t b, char *out, size_t n)
{
  snprintf(out, n, "%lu KB", (unsigned long)(b / 1024UL));
}

static void dibujarBarra(lgfx::LGFXBase *g, int16_t x, int16_t y, uint64_t usado, uint64_t total)
{
  const Tema &t = T();
  const uint8_t pct = total ? (uint8_t)(usado * 100ULL / total) : 0;
  g->fillRoundRect(x, y, BARRA_W, BARRA_H, BARRA_H / 2, t.superficieAlt);
  g->fillRoundRect(x, y, (int32_t)BARRA_W * pct / 100, BARRA_H, BARRA_H / 2, t.acento);

  char p[8];
  snprintf(p, sizeof(p), "%u%%", pct);
  g->setFont(&fonts::Font2);
  g->setTextDatum(middle_left);
  g->setTextColor(t.textoTenue);
  g->drawString(p, x + BARRA_W + 10, y + BARRA_H / 2);
}

static void dibujarFila(lgfx::LGFXBase *g, const Fila &f, int16_t y)
{
  const Tema &t = T();

  g->setFont(&fonts::Font4);
  g->setTextDatum(top_left);
  g->setTextColor(t.texto);
  g->drawString(f.etiqueta, GAP + 4, y);

  if (!f.disponible) {
    g->setFont(&fonts::Font2);
    g->setTextColor(t.textoTenue);
    g->setTextDatum(middle_left);
    g->drawString("no disponible", GAP + 4, y + 30);
    return;
  }

  char u[16], tt[16];
  textoKB(f.usado, u, sizeof(u));
  textoKB(f.total, tt, sizeof(tt));
  char linea[40];
  snprintf(linea, sizeof(linea), "%s de %s", u, tt);

  g->setFont(&fonts::Font2);
  g->setTextDatum(top_left);
  g->setTextColor(t.textoTenue);
  g->drawString(linea, GAP + 4, y + 22);

  dibujarBarra(g, GAP + 4, y + 22, f.usado, f.total);
}

static void dibujar()
{
  const Tema &t = T();
  lgfx::LGFXBase *g = pantalla();
  g->fillRect(0, ZONA_Y, ANCHO, ALTO - ZONA_Y, t.fondo);

  const uint32_t heapUsado = ESP.getHeapSize() - ESP.getFreeHeap();
  const Fila filas[] = {
    { "RAM (heap)",   heapUsado,            ESP.getHeapSize(),      true },
    { "PSRAM",        ESP.getPsramSize() ? ESP.getPsramSize() - ESP.getFreePsram() : 0,
                       ESP.getPsramSize(), ESP.getPsramSize() > 0 },
    { "Flash (programa)", ESP.getSketchSize(), ESP.getSketchSize() + ESP.getFreeSketchSpace(), true },
    { "LittleFS",     fsDisponible() ? LittleFS.usedBytes() : 0,
                       fsDisponible() ? LittleFS.totalBytes() : 0, fsDisponible() },
    { "Tarjeta SD",   sdMontada() ? SD.usedBytes() : 0,
                       sdMontada() ? SD.totalBytes() : 0, sdMontada() },
  };

  int16_t y = ZONA_Y + 6;
  for (const Fila &f : filas) {
    dibujarFila(g, f, y);
    y += FILA_ALTO;
  }

  pantallaEmpujar(0, ZONA_Y, ANCHO, ALTO - ZONA_Y);
}

void screenSistemaEntrar()
{
  pantalla()->fillScreen(T().fondo);
  cabeceraDibujar("Sistema", "< Ajustes");
  dibujar();
  pantallaEmpujarTodo();
}

void screenSistemaLoop()
{
  if (cabeceraAtras()) irAPantalla(SCREEN_AJUSTES);
}

void screenSistemaSalir() {}
