#include "Drivers/teclado.h"
#include <BleKeyboard.h>
#include <NimBLEDevice.h>

static BleKeyboard s_teclado(BLE_NOMBRE, "Tesla Lab", 100);

void tecladoIniciar()
{
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

void tecladoAtajo(std::initializer_list<uint8_t> teclas)
{
  if (!listo()) return;
  for (uint8_t k : teclas) s_teclado.press(k);
  delay(30);
  s_teclado.releaseAll();
}

void tecladoAbrirApp(const char *nombre)
{
  if (!listo()) return;
  s_teclado.press(TECLA_WIN);
  delay(30);
  s_teclado.releaseAll();
  delay(300);
  s_teclado.print(nombre);
  delay(500);
  s_teclado.write(KEY_RETURN);
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
