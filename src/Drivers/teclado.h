#ifndef PURPLE_DECK_DRIVERS_TECLADO_H_
#define PURPLE_DECK_DRIVERS_TECLADO_H_
#include <Arduino.h>
#include <initializer_list>

enum MediaTecla : uint8_t {
  MEDIA_VOL_UP = 0, MEDIA_VOL_DOWN, MEDIA_MUTE,
  MEDIA_PLAY_PAUSA, MEDIA_SIGUIENTE, MEDIA_ANTERIOR
};

#define TECLA_WIN    0x83
#define TECLA_SHIFT  0x81
#define TECLA_CTRL   0x80
#define TECLA_ALT    0x82

void tecladoIniciar();
bool tecladoConectado();
void tecladoMedia(MediaTecla m);
void tecladoAtajo(std::initializer_list<uint8_t> teclas);
void tecladoAbrirApp(const char *nombre);

#endif  // PURPLE_DECK_DRIVERS_TECLADO_H_
