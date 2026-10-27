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

// Teclas especiales (mismos codigos que BleKeyboard.h)
#define TECLA_TAB    0xB3
#define TECLA_ENTER  0xB0
#define TECLA_HOME   0xD2
#define TECLA_ARRIBA 0xDA
#define TECLA_ABAJO  0xD9
#define TECLA_IZQ    0xD8
#define TECLA_DER    0xD7
#define TECLA_F12    0xCD
#define TECLA_F5     0xC6
#define TECLA_F6     0xC7
#define TECLA_F7     0xC8
#define TECLA_F8     0xC9
#define TECLA_F9     0xCA
#define TECLA_F11    0xCC

// Numpad: BleKeyboard manda (k - 136) como codigo HID crudo para k >= 136.
// HID: Keypad 1 = 0x59, Keypad 3 = 0x5B, Keypad 7 = 0x5F
#define TECLA_NUM0   (136 + 0x62)
#define TECLA_NUM1   (136 + 0x59)
#define TECLA_NUM2   (136 + 0x5A)
#define TECLA_NUM3   (136 + 0x5B)
#define TECLA_NUM4   (136 + 0x5C)
#define TECLA_NUM5   (136 + 0x5D)
#define TECLA_NUM6   (136 + 0x5E)
#define TECLA_NUM7   (136 + 0x5F)
#define TECLA_NUM8   (136 + 0x60)
#define TECLA_NUM9   (136 + 0x61)

// Distribucion del teclado configurada en Windows. El deck manda posiciones
// de tecla, no caracteres: para que un simbolo salga bien hay que mandar la
// tecla que lo produce en la distribucion activa de la PC.
enum Distribucion : uint8_t { DIST_US = 0, DIST_LATAM, DIST_ES, NUM_DIST };

#define TECLA_ENYE   (136 + 0x33)   // la tecla de la n con tilde (';' en US)

void        tecladoDistribucion(uint8_t d);   // cambia y guarda en NVS
uint8_t     tecladoDistribucionActual();
const char *tecladoDistribucionNombre();

void tecladoIniciar();
bool tecladoConectado();
void tecladoMedia(MediaTecla m);
void tecladoAtajo(std::initializer_list<uint8_t> teclas);
void tecladoAtajoLista(const uint8_t *teclas, uint8_t n);   // igual, con arreglo (espacios editables)
void tecladoAbrirApp(const char *nombre);
void tecladoEscribir(const char *texto, bool enter);   // escribe texto (y Enter opcional)

// --- Bluetooth: estado y re-emparejado -------------------------------------
#define BLE_NOMBRE "Purple Deck"

bool        tecladoEmparejado();     // true si hay algun emparejamiento guardado
const char *tecladoDireccion();      // MAC del dispositivo conectado, o nullptr
void        tecladoReemparejar();    // desconecta, borra emparejamientos y vuelve a anunciarse

#endif  // PURPLE_DECK_DRIVERS_TECLADO_H_
