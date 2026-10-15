#ifndef PURPLE_DECK_ESTATICOS_TEMAS_H_
#define PURPLE_DECK_ESTATICOS_TEMAS_H_
// ---------------------------------------------------------------------------
//  Estaticos/temas.h - paletas de color del deck
//
//  Para agregar un tema: copia una entrada en TEMAS[] (temas.cpp) y listo,
//  NUM_TEMAS se calcula solo. Cambia el tema vivo con temaUsar(i).
// ---------------------------------------------------------------------------
#include <Arduino.h>

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

struct Tema {
  const char *nombre;
  uint16_t fondo;         // fondo de pantalla
  uint16_t superficie;    // tarjetas
  uint16_t superficieAlt; // tarjetas resaltadas / boton presionado
  uint16_t primario;      // color base de los botones
  uint16_t acento;        // detalles: segundero, barras, horas
  uint16_t acentoAlt;     // acento secundario
  uint16_t borde;
  uint16_t texto;
  uint16_t textoTenue;
  uint16_t textoSobreAcento;
};

extern const Tema TEMAS[];
extern const uint8_t NUM_TEMAS;

const Tema &T();                  // tema activo
void temaUsar(uint8_t indice);
uint8_t temaIndice();

#endif  // PURPLE_DECK_ESTATICOS_TEMAS_H_
