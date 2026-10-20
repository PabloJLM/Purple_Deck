#ifndef PURPLE_DECK_ESTATICOS_REJILLA_H_
#define PURPLE_DECK_ESTATICOS_REJILLA_H_
// ---------------------------------------------------------------------------
//  Estaticos/rejilla.h - motor de rejilla de botones
//
//  Lo usan el menu principal y todos los submenus: cada pantalla solo declara
//  su tabla de Item y una RejillaCfg, y el motor se encarga del layout, el
//  dibujo, el toque y la navegacion.
//
//  Dos modos:
//    - normal    (menu principal): los items llenan la pantalla en 'cols'
//                columnas.
//    - paginada  (submenus): rejilla fija de 3x3 donde
//                  abajo-izquierda = Atras (a la pantalla 'atras')
//                  abajo-derecha   = Siguiente (solo si hay mas de una pagina)
//                y los 7 huecos restantes llevan items; si hay mas de 7, se
//                reparten en paginas.
//
//  Etiquetas: solo ASCII (las fuentes del display no llevan acentos) y un
//  '\n' parte la etiqueta en dos lineas.
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include "config.h"

enum ItemTipo : uint8_t {
  ITEM_ACCION = 0,     // ejecuta una Accion (Drivers/acciones.h)
  ITEM_PANTALLA        // navega a otra pantalla (enum Screen)
};

struct Item {
  const char *etiqueta;            // texto principal
  const char *sub;                 // texto secundario fijo (o nullptr)
  const char *icono;               // ruta en LittleFS, p.ej. "/macros_1.png" (o nullptr)
  uint8_t     tipo;                // ItemTipo
  uint8_t     valor;               // Accion o Screen, segun 'tipo'
  const char *(*subDin)();         // texto secundario dinamico (o nullptr)
};

// Atajos para escribir las tablas
#define IT_ACC(e, s, ic, a)      { e, s,       ic, ITEM_ACCION,   (uint8_t)(a), nullptr }
#define IT_ACC_DIN(e, ic, a, f)  { e, nullptr, ic, ITEM_ACCION,   (uint8_t)(a), f       }
#define IT_PANT(e, s, ic, p)     { e, s,       ic, ITEM_PANTALLA, (uint8_t)(p), nullptr }
#define IT_PANT_DIN(e, ic, p, f) { e, nullptr, ic, ITEM_PANTALLA, (uint8_t)(p), f       }

#define REJILLA_MAX_ITEMS 28          // 4 paginas de 7

struct RejillaCfg {
  const Item *items;
  uint8_t     n;          // cuantos items
  uint8_t     cols;       // columnas (modo normal); la paginada siempre usa 3
  bool        paginada;   // true = 3x3 con Atras / Siguiente (ver arriba)
  Screen      atras;      // a donde lleva el boton Atras
  bool        anterior = false;  // true: abajo-izquierda es 'Ant' (pagina anterior) en vez de Atras
};

void rejillaEntrar(const RejillaCfg &c);   // reserva sprite y dibuja
void rejillaLoop();                        // atiende el toque
void rejillaSalir();                       // libera el sprite
void rejillaRedibujar(bool limpiar);       // repinta (limpiar = borra la pantalla antes)

#endif  // PURPLE_DECK_ESTATICOS_REJILLA_H_
