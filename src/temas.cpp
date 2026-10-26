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
  {
    /* nombre           */ "Light",
    /* background       */ rgb565( 73,  73,  73),   // #494949  ->  0x4A49
    /* surface          */ rgb565( 91,  91,  91),   // #5B5B5B  ->  0x5ACB
    /* surfaceAlt       */ rgb565(109, 109, 109),   // #6D6D6D  ->  0x6B6D
    /* primary          */ rgb565(154, 154, 154),   // #9A9A9A  ->  0x9CD3
    /* accent           */ rgb565(192, 192, 192),   // #C0C0C0  ->  0xC618
    /* text             */ rgb565(255, 255, 255),   // #FFFFFF  ->  0xFFFF
    /* textMuted        */ rgb565(173, 173, 173),   // #ADADAD  ->  0xAD75
    /* textOnAccent     */ rgb565( 18,  12,  24),   // #120C18  ->  0x1063
    /* border           */ rgb565(129, 129, 129),   // #818181  ->  0x8410
    /* accentAlt        */ rgb565(192, 192, 192),   // #C0C0C0  ->  0xC618
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
