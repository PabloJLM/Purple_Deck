#ifndef PURPLE_DECK_DRIVERS_SLOTS_UTIL_H_
#define PURPLE_DECK_DRIVERS_SLOTS_UTIL_H_
// ---------------------------------------------------------------------------
//  Drivers/slots_util.h - funciones puras (sin Arduino) de los espacios
//  editables: partir una linea de comando y convertir "ctrl+shift+p" a codigos
//  de tecla. Estan aparte para poder probarlas en la PC con g++.
// ---------------------------------------------------------------------------
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Parte 's' (se modifica) en palabras separadas por espacios. Lo que va entre
// "comillas" es una sola palabra (puede llevar espacios o estar vacio).
// Devuelve cuantas palabras hay, -1 si queda una comilla sin cerrar, -2 si
// hay mas de 'max'.
inline int slotTokenizar(char *s, char **tok, int max)
{
  int n = 0;
  while (*s) {
    while (*s == ' ') s++;
    if (!*s) break;
    if (n >= max) return -2;
    if (*s == '"') {
      s++;
      tok[n++] = s;
      while (*s && *s != '"') s++;
      if (!*s) return -1;
      *s++ = '\0';
    } else {
      tok[n++] = s;
      while (*s && *s != ' ') s++;
      if (*s) *s++ = '\0';
    }
  }
  return n;
}

struct SlotNombreTecla { const char *nombre; uint8_t codigo; };

// Mismos codigos que BleKeyboard.h
static const SlotNombreTecla SLOT_TABLA_TECLAS[] = {
  { "ctrl", 0x80 }, { "control", 0x80 },
  { "shift", 0x81 },
  { "alt", 0x82 },
  { "win", 0x83 }, { "gui", 0x83 }, { "super", 0x83 }, { "windows", 0x83 },
  { "altgr", 0x86 },
  { "enter", 0xB0 }, { "return", 0xB0 },
  { "esc", 0xB1 }, { "escape", 0xB1 },
  { "backspace", 0xB2 },
  { "tab", 0xB3 },
  { "space", 0x20 },
  { "insert", 0xD1 },
  { "delete", 0xD4 }, { "del", 0xD4 },
  { "home", 0xD2 },
  { "end", 0xD5 },
  { "pageup", 0xD3 }, { "pgup", 0xD3 },
  { "pagedown", 0xD6 }, { "pgdn", 0xD6 },
  { "right", 0xD7 }, { "left", 0xD8 }, { "down", 0xD9 }, { "up", 0xDA },
  { "f1", 0xC2 }, { "f2", 0xC3 }, { "f3", 0xC4 }, { "f4", 0xC5 },
  { "f5", 0xC6 }, { "f6", 0xC7 }, { "f7", 0xC8 }, { "f8", 0xC9 },
  { "f9", 0xCA }, { "f10", 0xCB }, { "f11", 0xCC }, { "f12", 0xCD },
};

// Convierte "ctrl+shift+p" en codigos de tecla (en 'out', maximo 'max').
// Devuelve cuantos son, o 0 si hay algo que no se entiende.
// Una tecla de un solo caracter se manda tal cual (las letras en minuscula);
// para escribir el signo + como tecla: "ctrl++".
inline int slotParsearTeclas(const char *txt, uint8_t *out, int max)
{
  char buf[64];
  if (!txt || strlen(txt) >= sizeof(buf)) return 0;
  strcpy(buf, txt);

  int n = 0;
  char *p = buf;
  while (*p) {
    char *fin = p;
    if (p[0] == '+' && p[1] == '\0') {
      fin = p + 1;                       // el signo + como tecla
    } else {
      while (*fin && *fin != '+') fin++;
    }
    const bool ultimo = (*fin == '\0');
    if (!ultimo) *fin = '\0';

    char tok[16];
    const size_t len = strlen(p);
    if (len == 0 || len >= sizeof(tok)) return 0;
    for (size_t i = 0; i <= len; i++) {
      char c = p[i];
      tok[i] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
    }

    if (n >= max) return 0;
    if (len == 1) {
      out[n++] = (uint8_t)tok[0];
    } else {
      bool ok = false;
      for (size_t i = 0; i < sizeof(SLOT_TABLA_TECLAS) / sizeof(SLOT_TABLA_TECLAS[0]); i++) {
        if (strcmp(tok, SLOT_TABLA_TECLAS[i].nombre) == 0) {
          out[n++] = SLOT_TABLA_TECLAS[i].codigo;
          ok = true;
          break;
        }
      }
      if (!ok) return 0;
    }
    if (ultimo) break;
    p = fin + 1;
    if (*p == '\0') return 0;            // termina en "+" suelto: "ctrl+"
  }
  return n;
}

#endif  // PURPLE_DECK_DRIVERS_SLOTS_UTIL_H_
