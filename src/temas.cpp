#include "Estaticos/temas.h"

const Tema TEMAS[] = {
  {
    /* nombre           */ "Afton",
    /* fondo            */ rgb565( 19,  10,  28),   // #130A1C
    /* superficie       */ rgb565( 36,  18,  51),   // #241233
    /* superficieAlt    */ rgb565( 50,  25,  74),   // #32194A
    /* primario         */ rgb565(109,  40, 168),   // #6D28A8 
    /* acento           */ rgb565(201, 162,  39),   // #C9A227 
    /* acentoAlt        */ rgb565(138, 154,  75),   // #8A9A4B
    /* borde            */ rgb565( 74,  42, 107),   // #4A2A6B
    /* texto            */ rgb565(243, 233, 216),   // #F3E9D8 
    /* textoTenue       */ rgb565(155, 130, 190),   // #9B82BE
    /* textoSobreAcento */ rgb565( 25,  14,  36),
  },
};

const uint8_t NUM_TEMAS = sizeof(TEMAS) / sizeof(TEMAS[0]);

static uint8_t s_tema = 0;

const Tema &T() { return TEMAS[s_tema]; }
uint8_t temaIndice() { return s_tema; }

void temaUsar(uint8_t indice)
{
  if (indice < NUM_TEMAS) s_tema = indice;
}
