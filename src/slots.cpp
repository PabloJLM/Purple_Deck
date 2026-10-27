#include "Drivers/slots.h"
#include "Drivers/slots_util.h"
#include "Drivers/acciones.h"
#include "Drivers/teclado.h"
#include "Estaticos/rejilla.h"
#include <Preferences.h>

// ---------------------------------------------------------------------------
//  Modelo: cada slot tiene un tipo y tres textos.
//    accion  : valor = clave de una accion existente (ver 'slot actions')
//    teclas  : valor = combinacion, p.ej. "ctrl+shift+p"
//    texto   : valor = texto que se escribe en la PC
//    comando : como texto, pero con Enter al final
//  Los textos son ASCII y no pueden llevar '|' (es el separador al guardar).
// ---------------------------------------------------------------------------
enum SlotTipo : uint8_t { SLOT_VACIO = 0, SLOT_ACCION, SLOT_TECLAS, SLOT_TEXTO, SLOT_COMANDO };

#define LEN_ETIQUETA 24
#define LEN_VALOR    96
#define LEN_ICONO    32

struct Slot {
  uint8_t tipo;
  char    etiqueta[LEN_ETIQUETA];
  char    valor[LEN_VALOR];
  char    icono[LEN_ICONO];
};

static const char *NOMBRE_TIPO[] = { "vacio", "accion", "teclas", "texto", "comando" };

static Slot     s_slots[SLOTS_N];
static uint32_t s_version = 1;

// --- persistencia ----------------------------------------------------------
static void clave(uint8_t i, char *k) { snprintf(k, 8, "s%u", (unsigned)i); }

static void guardar(uint8_t i)
{
  const Slot &s = s_slots[i];
  Preferences p;
  p.begin("slots", false);
  char k[8];
  clave(i, k);
  if (s.tipo == SLOT_VACIO) {
    p.remove(k);
  } else {
    String v = String(s.tipo) + "|" + s.etiqueta + "|" + s.valor + "|" + s.icono;
    p.putString(k, v);
  }
  p.end();
}

static void cargar(uint8_t i, const String &v)
{
  Slot &s = s_slots[i];
  memset(&s, 0, sizeof(s));
  // tipo|etiqueta|valor|icono
  const int a = v.indexOf('|');
  const int b = v.indexOf('|', a + 1);
  const int c = v.indexOf('|', b + 1);
  if (a < 0 || b < 0 || c < 0) return;
  const int tipo = v.substring(0, a).toInt();
  if (tipo <= SLOT_VACIO || tipo > SLOT_COMANDO) return;
  s.tipo = (uint8_t)tipo;
  strlcpy(s.etiqueta, v.substring(a + 1, b).c_str(), sizeof(s.etiqueta));
  strlcpy(s.valor,    v.substring(b + 1, c).c_str(), sizeof(s.valor));
  strlcpy(s.icono,    v.substring(c + 1).c_str(),    sizeof(s.icono));
}

void slotsIniciar()
{
  Preferences p;
  p.begin("slots", true);
  for (uint8_t i = 0; i < SLOTS_N; i++) {
    char k[8];
    clave(i, k);
    cargar(i, p.getString(k, ""));
  }
  p.end();
  s_version++;
}

uint32_t slotsVersion() { return s_version; }

// --- ejecucion -------------------------------------------------------------
void slotEjecutar(uint8_t i)
{
  if (i >= SLOTS_N) return;
  const Slot &s = s_slots[i];
  switch (s.tipo) {
    case SLOT_ACCION: {
      Accion a;
      if (accionPorClave(s.valor, &a)) accionEjecutar(a);
      else Serial.printf("[slot] %u: la accion '%s' ya no existe\n", (unsigned)(i + 1), s.valor);
      break;
    }
    case SLOT_TECLAS: {
      uint8_t t[6];
      const int n = slotParsearTeclas(s.valor, t, 6);
      if (n > 0) tecladoAtajoLista(t, (uint8_t)n);
      break;
    }
    case SLOT_TEXTO:   tecladoEscribir(s.valor, false); break;
    case SLOT_COMANDO: tecladoEscribir(s.valor, true);  break;
    default: break;
  }
}

// --- item para los menus ------------------------------------------------------
bool slotItem(uint8_t i, Item *out)
{
  if (i >= SLOTS_N || s_slots[i].tipo == SLOT_VACIO) return false;
  *out = Item{ s_slots[i].etiqueta, nullptr,
               s_slots[i].icono[0] ? s_slots[i].icono : nullptr,
               ITEM_ACCION, (uint8_t)(ACC_SLOT_1 + i), nullptr };
  return true;
}

// --- comandos de la terminal --------------------------------------------------
// Respuesta pensada para maquinas y personas: lineas "SLOT ..." / "ACT ..." y
// al final "OK" o "ERR <motivo>".
static void ok()                { Serial.println(F("OK")); }
static void err(const char *m)  { Serial.print(F("ERR ")); Serial.println(m); }

static void ayuda()
{
  Serial.println(F("  slot list                              muestra las 27 macros (3 menus x 9)"));
  Serial.println(F("  slot actions                           acciones que se pueden asignar"));
  Serial.println(F("  slot set N tipo \"etiqueta\" \"valor\" [\"icono\"]"));
  Serial.println(F("       tipo: accion | teclas | texto | comando"));
  Serial.println(F("       accion  valor = clave de 'slot actions'"));
  Serial.println(F("       teclas  valor = p.ej. \"ctrl+shift+p\"  (ctrl shift alt altgr win, f1-f12,"));
  Serial.println(F("                       enter tab esc space backspace delete home end"));
  Serial.println(F("                       pageup pagedown up down left right)"));
  Serial.println(F("       texto   valor = texto que escribe;  comando = texto + Enter"));
  Serial.println(F("       en la etiqueta, \\n parte el texto en dos lineas"));
  Serial.println(F("  slot clear N                           vacia la macro N (1-27)"));
  Serial.println(F("  slot test N                            ejecuta la macro N"));
  ok();
}

static void listar()
{
  for (uint8_t i = 0; i < SLOTS_N; i++) {
    const Slot &s = s_slots[i];
    Serial.printf("SLOT %u|%s|", (unsigned)(i + 1), NOMBRE_TIPO[s.tipo]);
    for (const char *c = s.etiqueta; *c; c++) {
      if (*c == '\n') Serial.print(F("\\n"));
      else            Serial.write(*c);
    }
    Serial.printf("|%s|%s\n", s.valor, s.icono);
  }
  ok();
}

static void acciones()
{
  for (uint8_t i = 0; i < accionesCantidad(); i++) {
    char k[40];
    const char *g, *n, *a;
    if (!accionInfo(i, k, sizeof(k), &g, &n, &a)) continue;
    Serial.printf("ACT %s|%s|%s|%s\n", k, g, n, a);
  }
  ok();
}

static bool sinBarra(const char *t) { return strchr(t, '|') == nullptr; }

static void ponerSlot(char **t, int n)
{
  // t[0]="slot" t[1]="set" t[2]=N t[3]=tipo t[4]=etiqueta t[5]=valor [t[6]=icono]
  if (n < 6 || n > 7) { err("uso: slot set N tipo \"etiqueta\" \"valor\" [\"icono\"]"); return; }
  const int num = atoi(t[2]);
  if (num < 1 || num > SLOTS_N) { err("N debe estar entre 1 y 27"); return; }

  uint8_t tipo = SLOT_VACIO;
  for (uint8_t k = SLOT_ACCION; k <= SLOT_COMANDO; k++)
    if (strcmp(t[3], NOMBRE_TIPO[k]) == 0) tipo = k;
  if (tipo == SLOT_VACIO) { err("tipo: accion, teclas, texto o comando"); return; }

  const char *etiq = t[4], *valor = t[5], *icono = (n == 7) ? t[6] : "";
  if (!etiq[0])  { err("la etiqueta no puede estar vacia"); return; }
  if (!valor[0]) { err("el valor no puede estar vacio"); return; }
  if (strlen(etiq)  >= LEN_ETIQUETA) { err("etiqueta de maximo 23 caracteres"); return; }
  if (strlen(valor) >= LEN_VALOR)    { err("valor de maximo 95 caracteres"); return; }
  if (strlen(icono) >= LEN_ICONO)    { err("icono de maximo 31 caracteres"); return; }
  if (!sinBarra(etiq) || !sinBarra(valor) || !sinBarra(icono)) { err("no se permite el caracter |"); return; }

  if (tipo == SLOT_ACCION) {
    Accion a;
    if (!accionPorClave(valor, &a)) { err("accion desconocida (mira 'slot actions')"); return; }
  } else if (tipo == SLOT_TECLAS) {
    uint8_t tmp[6];
    if (slotParsearTeclas(valor, tmp, 6) == 0) { err("combinacion de teclas no valida"); return; }
  }
  if (icono[0] && icono[0] != '/') { err("el icono es una ruta que empieza con /"); return; }

  Slot &s = s_slots[num - 1];
  memset(&s, 0, sizeof(s));
  s.tipo = tipo;
  // "\n" escrito como dos caracteres pasa a ser un salto de linea
  size_t o = 0;
  for (const char *c = etiq; *c && o < sizeof(s.etiqueta) - 1; c++) {
    if (c[0] == '\\' && c[1] == 'n') { s.etiqueta[o++] = '\n'; c++; }
    else                             { s.etiqueta[o++] = *c; }
  }
  strlcpy(s.valor, valor, sizeof(s.valor));
  strlcpy(s.icono, icono, sizeof(s.icono));
  guardar(num - 1);
  s_version++;
  ok();
}

static void vaciarSlot(char **t, int n)
{
  if (n != 3) { err("uso: slot clear N"); return; }
  const int num = atoi(t[2]);
  if (num < 1 || num > SLOTS_N) { err("N debe estar entre 1 y 27"); return; }
  memset(&s_slots[num - 1], 0, sizeof(Slot));
  guardar(num - 1);
  s_version++;
  ok();
}

static void probarSlot(char **t, int n)
{
  if (n != 3) { err("uso: slot test N"); return; }
  const int num = atoi(t[2]);
  if (num < 1 || num > SLOTS_N) { err("N debe estar entre 1 y 27"); return; }
  if (s_slots[num - 1].tipo == SLOT_VACIO) { err("esa macro esta vacia"); return; }
  slotEjecutar(num - 1);
  ok();
}

bool slotsComando(const String &linea)
{
  if (!(linea == "slot" || linea.startsWith("slot "))) return false;

  char buf[256];
  if (linea.length() >= sizeof(buf)) { err("linea demasiado larga"); return true; }
  strlcpy(buf, linea.c_str(), sizeof(buf));

  char *t[8];
  const int n = slotTokenizar(buf, t, 8);
  if (n == -1) { err("comilla sin cerrar"); return true; }
  if (n < 0)   { err("demasiados argumentos"); return true; }

  if      (n == 1)                      ayuda();
  else if (!strcmp(t[1], "help"))       ayuda();
  else if (!strcmp(t[1], "list"))       listar();
  else if (!strcmp(t[1], "actions"))    acciones();
  else if (!strcmp(t[1], "set"))        ponerSlot(t, n);
  else if (!strcmp(t[1], "clear"))      vaciarSlot(t, n);
  else if (!strcmp(t[1], "test"))       probarSlot(t, n);
  else err("subcomando desconocido (escribe 'slot help')");
  return true;
}
