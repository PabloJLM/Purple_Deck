#include "Drivers/teclado.h"
#include <BleKeyboard.h>
#include <NimBLEDevice.h>
#include <Preferences.h>

static BleKeyboard s_teclado(BLE_NOMBRE, "Tesla Lab", 100);

// --- distribuciones ---------------------------------------------------------
// Solo se listan los simbolos; letras, numeros y espacio estan en la misma
// tecla en las tres. 'uso' es el codigo HID de la tecla fisica (el nombre
// entre parentesis es lo que esa tecla escribe en US).
#define M_SHIFT  0x01
#define M_ALTGR  0x02   // Alt derecho
#define M_MUERTA 0x04   // tecla muerta: hay que mandar espacio despues

struct Simbolo { char c; uint8_t uso; uint8_t mods; };

static const Simbolo LATAM[] = {
  { '!', 0x1E, M_SHIFT }, { '"', 0x1F, M_SHIFT }, { '#', 0x20, M_SHIFT },
  { '$', 0x21, M_SHIFT }, { '%', 0x22, M_SHIFT }, { '&', 0x23, M_SHIFT },
  { '/', 0x24, M_SHIFT }, { '(', 0x25, M_SHIFT }, { ')', 0x26, M_SHIFT },
  { '=', 0x27, M_SHIFT },
  { '\'', 0x2D, 0 },       { '?', 0x2D, M_SHIFT }, { '\\', 0x2D, M_ALTGR },   // (-)
  { '|', 0x35, 0 },                                                        // (`)
  { '+', 0x30, 0 },        { '*', 0x30, M_SHIFT }, { '~', 0x30, M_ALTGR },     // (])
  { '{', 0x34, 0 },        { '[', 0x34, M_SHIFT }, { '^', 0x34, M_ALTGR | M_MUERTA },  // (')
  { '}', 0x31, 0 },        { ']', 0x31, M_SHIFT }, { '`', 0x31, M_ALTGR | M_MUERTA },  // (\)
  { '-', 0x38, 0 },        { '_', 0x38, M_SHIFT },                         // (/)
  { ',', 0x36, 0 },        { ';', 0x36, M_SHIFT },
  { '.', 0x37, 0 },        { ':', 0x37, M_SHIFT },
  { '<', 0x64, 0 },        { '>', 0x64, M_SHIFT },                         // tecla ISO junto a Z
  { '@', 0x14, M_ALTGR },                                                  // AltGr+Q
};

static const Simbolo ESPANA[] = {
  { '!', 0x1E, M_SHIFT }, { '"', 0x1F, M_SHIFT }, { '#', 0x20, M_ALTGR },
  { '$', 0x21, M_SHIFT }, { '%', 0x22, M_SHIFT }, { '&', 0x23, M_SHIFT },
  { '/', 0x24, M_SHIFT }, { '(', 0x25, M_SHIFT }, { ')', 0x26, M_SHIFT },
  { '=', 0x27, M_SHIFT },
  { '|', 0x1E, M_ALTGR }, { '@', 0x1F, M_ALTGR }, { '~', 0x21, M_ALTGR },
  { '\'', 0x2D, 0 },       { '?', 0x2D, M_SHIFT },                         // (-)
  { '`', 0x2F, M_MUERTA }, { '^', 0x2F, M_SHIFT | M_MUERTA }, { '[', 0x2F, M_ALTGR },  // ([)
  { '+', 0x30, 0 },        { '*', 0x30, M_SHIFT }, { ']', 0x30, M_ALTGR },     // (])
  { '{', 0x34, M_ALTGR },                                                  // (')
  { '}', 0x31, M_ALTGR },                                                  // (\)
  { '\\', 0x35, M_ALTGR },                                                 // (`)
  { '-', 0x38, 0 },        { '_', 0x38, M_SHIFT },                         // (/)
  { ',', 0x36, 0 },        { ';', 0x36, M_SHIFT },
  { '.', 0x37, 0 },        { ':', 0x37, M_SHIFT },
  { '<', 0x64, 0 },        { '>', 0x64, M_SHIFT },
};

struct DistDef { const char *nombre; const Simbolo *tabla; uint8_t n; };

static const DistDef DISTS[NUM_DIST] = {
  { "EE.UU.",        nullptr, 0 },   // US: BleKeyboard ya escribe en US
  { "Latinoamerica", LATAM,  sizeof(LATAM)  / sizeof(LATAM[0]) },
  { "Espana",        ESPANA, sizeof(ESPANA) / sizeof(ESPANA[0]) },
};

static uint8_t s_dist = DIST_US;

void tecladoDistribucion(uint8_t d)
{
  if (d >= NUM_DIST) return;
  s_dist = d;
  Preferences p;
  p.begin("teclado", false);
  p.putUChar("dist", d);
  p.end();
  Serial.printf("[teclado] distribucion: %s\n", DISTS[d].nombre);
}

uint8_t     tecladoDistribucionActual() { return s_dist; }
const char *tecladoDistribucionNombre() { return DISTS[s_dist].nombre; }

static const Simbolo *buscarSimbolo(char c)
{
  const DistDef &d = DISTS[s_dist];
  for (uint8_t i = 0; i < d.n; i++)
    if (d.tabla[i].c == c) return &d.tabla[i];
  return nullptr;
}

static void escribirChar(char c)
{
  const Simbolo *sim = buscarSimbolo(c);
  if (!sim) { s_teclado.write((uint8_t)c); delay(12); return; }

  if (sim->mods & M_SHIFT) s_teclado.press(KEY_LEFT_SHIFT);
  if (sim->mods & M_ALTGR) s_teclado.press(KEY_RIGHT_ALT);
  s_teclado.press((uint8_t)(136 + sim->uso));
  delay(5);
  s_teclado.releaseAll();
  if (sim->mods & M_MUERTA) s_teclado.write((uint8_t)' ');
  delay(12);   // pausa entre teclas
}

static void escribirTexto(const char *t)
{
  while (*t) escribirChar(*t++);
}

void tecladoIniciar()
{
  Preferences p;
  p.begin("teclado", true);
  s_dist = p.getUChar("dist", DIST_US);
  p.end();
  if (s_dist >= NUM_DIST) s_dist = DIST_US;

  s_teclado.begin();
  // por defecto NimBLE NO vuelve a anunciarse sola tras una desconexion
  NimBLEServer *srv = NimBLEDevice::getServer();
  if (srv) srv->advertiseOnDisconnect(true);
}

bool tecladoConectado()
{
  return s_teclado.isConnected();
}

static bool listo()
{
  if (s_teclado.isConnected()) return true;
  Serial.println("[teclado] sin conexion Bluetooth con la PC");
  return false;
}

void tecladoMedia(MediaTecla m)
{
  if (!listo()) return;
  switch (m) {
    case MEDIA_VOL_UP:     s_teclado.write(KEY_MEDIA_VOLUME_UP);     break;
    case MEDIA_VOL_DOWN:   s_teclado.write(KEY_MEDIA_VOLUME_DOWN);   break;
    case MEDIA_MUTE:       s_teclado.write(KEY_MEDIA_MUTE);          break;
    case MEDIA_PLAY_PAUSA: s_teclado.write(KEY_MEDIA_PLAY_PAUSE);    break;
    case MEDIA_SIGUIENTE:  s_teclado.write(KEY_MEDIA_NEXT_TRACK);    break;
    case MEDIA_ANTERIOR:   s_teclado.write(KEY_MEDIA_PREVIOUS_TRACK); break;
  }
}

void tecladoAtajoLista(const uint8_t *teclas, uint8_t n)
{
  if (!listo()) return;
  for (uint8_t i = 0; i < n; i++) {
    const uint8_t k = teclas[i];
    const Simbolo *sim = (k < 128) ? buscarSimbolo((char)k) : nullptr;
    if (!sim) { s_teclado.press(k); continue; }
    if (sim->mods & M_SHIFT) s_teclado.press(KEY_LEFT_SHIFT);
    if (sim->mods & M_ALTGR) s_teclado.press(KEY_RIGHT_ALT);
    s_teclado.press((uint8_t)(136 + sim->uso));
  }
  delay(30);
  s_teclado.releaseAll();
}

void tecladoAtajo(std::initializer_list<uint8_t> teclas)
{
  tecladoAtajoLista(teclas.begin(), (uint8_t)teclas.size());
}

void tecladoAbrirApp(const char *nombre)
{
  if (!listo()) return;
  s_teclado.press(TECLA_WIN);
  delay(30);
  s_teclado.releaseAll();
  delay(300);
  escribirTexto(nombre);
  delay(500);
  s_teclado.write(KEY_RETURN);
}

void tecladoEscribir(const char *texto, bool enter)
{
  if (!listo()) return;
  escribirTexto(texto);
  if (enter) {
    delay(30);
    s_teclado.write(KEY_RETURN);
  }
}

bool tecladoEmparejado()
{
  return NimBLEDevice::getNumBonds() > 0;
}

const char *tecladoDireccion()
{
  static char b[24];
  NimBLEServer *srv = NimBLEDevice::getServer();
  if (!srv || srv->getConnectedCount() == 0) return nullptr;

  strncpy(b, srv->getPeerInfo((uint8_t)0).getAddress().toString().c_str(), sizeof(b) - 1);
  b[sizeof(b) - 1] = '\0';
  return b;
}

void tecladoReemparejar()
{
  NimBLEServer *srv = NimBLEDevice::getServer();
  if (srv && srv->getConnectedCount() > 0) srv->disconnect(srv->getPeerInfo((uint8_t)0));

  NimBLEDevice::deleteAllBonds();

  if (srv) srv->getAdvertising()->start();
  Serial.println("[teclado] bluetooth reiniciado: emparejamientos borrados, anunciandose de nuevo");
}
