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
    // NOTA: este bloque estaba mal -- los valores venian del generador en su
    // orden de roles (background, surface, surfaceAlt, primary, accent,
    // text, textMuted, textOnAccent, border, accentAlt), que no coincide con
    // el orden de los campos de Tema (fondo, superficie, superficieAlt,
    // primario, acento, acentoAlt, borde, texto, textoTenue,
    // textoSobreAcento). Resultado: "texto" terminaba con el valor de
    // "accentAlt" y "textoSobreAcento" con el de... en resumen, letras
    // invisibles. Mismos colores, reordenados a los campos correctos.
    /* nombre           */ "Light",
    /* fondo            */ rgb565( 73,  73,  73),   // #494949
    /* superficie       */ rgb565( 91,  91,  91),   // #5B5B5B
    /* superficieAlt    */ rgb565(109, 109, 109),   // #6D6D6D
    /* primario         */ rgb565(154, 154, 154),   // #9A9A9A
    /* acento           */ rgb565(192, 192, 192),   // #C0C0C0
    /* acentoAlt        */ rgb565(192, 192, 192),   // #C0C0C0
    /* borde            */ rgb565(129, 129, 129),   // #818181
    /* texto            */ rgb565(255, 255, 255),   // #FFFFFF
    /* textoTenue       */ rgb565(173, 173, 173),   // #ADADAD
    /* textoSobreAcento */ rgb565( 18,  12,  24),   // #120C18
  },
  {
    // Mismo arreglo de orden que "Light" -- ver nota arriba.
    /* nombre           */ "Cyberpunk",
    /* fondo            */ rgb565(123,  44, 191),   // #7B2CBF
    /* superficie       */ rgb565(141,  61, 210),   // #8D3DD2
    /* superficieAlt    */ rgb565(158,  90, 217),   // #9E5AD9
    /* primario         */ rgb565( 24, 217, 120),   // #18D978
    /* acento           */ rgb565(255,  46, 151),   // #FF2E97
    /* acentoAlt        */ rgb565(215,  72, 229),   // #D748E5
    /* borde            */ rgb565( 98, 147, 173),   // #6293AD
    /* texto            */ rgb565(240, 230, 255),   // #F0E6FF
    /* textoTenue       */ rgb565(187, 146, 226),   // #BB92E2
    /* textoSobreAcento */ rgb565( 18,  12,  24),   // #120C18
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
