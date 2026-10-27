#include "Drivers/menus.h"
#include "Drivers/slots.h"
#include "Drivers/slots_util.h"
#include "Estaticos/rejilla.h"
#include <Preferences.h>

// ---------------------------------------------------------------------------
//  Definicion de los menus
// ---------------------------------------------------------------------------
struct MenuDef {
  const char *clave;      // prefijo de las claves de sus botones
  const char *nombre;
  Screen      atras;
  bool        anterior;   // el menu principal usa 'Ant' en vez de 'Atras'
};

static const MenuDef MENUS[MENUS_N] = {
  { "principal", "Menu principal", SCREEN_MENU,   true  },
  { "macros",    "Macros",         SCREEN_MENU,   false },
  { "solid",     "SolidWorks",     SCREEN_MENU,   false },
  { "labview",   "LabVIEW",        SCREEN_MENU,   false },
  { "git",       "Git / VS Code",  SCREEN_MENU,   false },
  { "blender",   "Blender",        SCREEN_MENU,   false },
  { "candle",    "Candle CNC",     SCREEN_MENU,   false },
  { "mover",     "Candle - Mover", SCREEN_CANDLE, false },
  { "c1",        "Custom 1",       SCREEN_MENU,   false },
  { "c2",        "Custom 2",       SCREEN_MENU,   false },
  { "c3",        "Custom 3",       SCREEN_MENU,   false },
};

#define NOMBRE_MAX 16
static char s_nombres[3][NOMBRE_MAX];

const char *menuNombreCustom(uint8_t k) { return s_nombres[k < 3 ? k : 0]; }

// ---------------------------------------------------------------------------
//  Registro de botones
// ---------------------------------------------------------------------------
#define REG_MAX 128
#define CLAVE_LEN 32

struct Reg {
  char    clave[CLAVE_LEN];
  Item    item;
  uint8_t def;        // menu de fabrica
  int8_t  menu;       // menu actual; -1 oculto; -2 sin asignar (solo al armar)
  uint16_t orden;
};

static Reg      s_reg[REG_MAX];
static uint8_t  s_nreg = 0;

static Item     s_items[MENUS_N][REJILLA_MAX_ITEMS];
static RejillaCfg s_cfg[MENUS_N];
static uint32_t s_ver = 1;            // sube al reconstruir
static uint32_t s_visto[MENUS_N];     // version con la que se dibujo cada menu
static uint32_t s_slotsVer = 0;
static int8_t   s_abierto = -1;

static void slug(const char *e, char *o, size_t max)
{
  size_t n = 0;
  bool sep = false;
  for (; *e && n < max - 1; e++) {
    char c = *e;
    if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      if (sep && n) o[n++] = '_';
      sep = false;
      if (n < max - 1) o[n++] = c;
    } else if (c == '+') { sep = false; o[n++] = 'p'; }
    else if (c == '-')   { sep = false; o[n++] = 'm'; }
    else sep = true;
  }
  o[n] = '\0';
  if (!n) strlcpy(o, "x", max);
}

static int buscar(const char *clave)
{
  for (uint8_t i = 0; i < s_nreg; i++)
    if (!strcmp(s_reg[i].clave, clave)) return i;
  return -1;
}

static void agregar(uint8_t menu, const char *clave, const Item &it)
{
  if (s_nreg >= REG_MAX) return;
  char k[CLAVE_LEN];
  strlcpy(k, clave, sizeof(k));
  // claves repetidas dentro de un menu: sufijo numerico
  for (uint8_t d = 2; buscar(k) >= 0 && d < 20; d++) snprintf(k, sizeof(k), "%s%u", clave, (unsigned)d);
  Reg &r = s_reg[s_nreg++];
  strlcpy(r.clave, k, sizeof(r.clave));
  r.item = it;
  r.def = menu;
  r.menu = -2;
  r.orden = 0;
}

static void registrar(uint8_t menu, const Item *its, uint8_t n)
{
  for (uint8_t i = 0; i < n; i++) {
    char base[CLAVE_LEN], s[22];
    if (its[i].tipo == ITEM_PANTALLA && its[i].valor >= SCREEN_CUSTOM1 && its[i].valor <= SCREEN_CUSTOM3) {
      snprintf(base, sizeof(base), "principal/custom%u", (unsigned)(its[i].valor - SCREEN_CUSTOM1 + 1));
    } else {
      slug(its[i].etiqueta, s, sizeof(s));
      snprintf(base, sizeof(base), "%s/%s", MENUS[menu].clave, s);
    }
    agregar(menu, base, its[i]);
  }
}

static bool protegido(const Reg &r)
{
  // Ajustes nunca desaparece: sin el no se podria volver a configurar el deck
  return r.item.tipo == ITEM_PANTALLA && r.item.valor == SCREEN_AJUSTES;
}

static void partir(const String &lista, void (*fn)(const char *, void *), void *ctx)
{
  int a = 0;
  while (a < (int)lista.length()) {
    int b = lista.indexOf(',', a);
    if (b < 0) b = lista.length();
    if (b > a) { String t = lista.substring(a, b); fn(t.c_str(), ctx); }
    a = b + 1;
  }
}

struct CtxLista { int8_t menu; uint16_t pos; };

static void alAsignar(const char *k, void *c)
{
  CtxLista *x = (CtxLista *)c;
  const int i = buscar(k);
  if (i < 0 || s_reg[i].menu != -2) return;
  s_reg[i].menu = x->menu;
  s_reg[i].orden = x->pos++;
}

static void alOcultar(const char *k, void *)
{
  const int i = buscar(k);
  if (i >= 0 && s_reg[i].menu == -2) s_reg[i].menu = -1;
}

static void reconstruir()
{
  s_nreg = 0;
  uint8_t n;
  const Item *p;
  p = screenMenuItems(&n);           registrar(M_PRINCIPAL, p, n);
  p = screenMacrosItems(&n);         registrar(M_MACROS, p, n);
  p = screenSolidworksItems(&n);     registrar(M_SOLIDWORKS, p, n);
  p = screenLabviewItems(&n);        registrar(M_LABVIEW, p, n);
  p = screenGitItems(&n);            registrar(M_GIT, p, n);
  p = screenBlenderItems(&n);        registrar(M_BLENDER, p, n);
  p = screenCandleItems(&n);         registrar(M_CANDLE, p, n);
  p = screenCandleMoverItems(&n);    registrar(M_CANDLE_MOVER, p, n);
  for (uint8_t i = 0; i < SLOTS_N; i++) {
    Item it;
    if (!slotItem(i, &it)) continue;
    char k[CLAVE_LEN];
    snprintf(k, sizeof(k), "c%u/%u", (unsigned)(i / SLOTS_POR_MENU + 1), (unsigned)(i % SLOTS_POR_MENU + 1));
    agregar(M_CUSTOM1 + i / SLOTS_POR_MENU, k, it);
  }

  // distribucion guardada
  Preferences pr;
  pr.begin("menus", true);
  for (uint8_t m = 0; m < MENUS_N; m++) {
    char k[6];
    snprintf(k, sizeof(k), "l%u", (unsigned)m);
    CtxLista c = { (int8_t)m, 0 };
    partir(pr.getString(k, ""), alAsignar, &c);
  }
  partir(pr.getString("oc", ""), alOcultar, nullptr);
  pr.end();

  // lo que no esta en ninguna lista vuelve a su menu de fabrica, al final
  uint16_t extra = 1000;
  for (uint8_t i = 0; i < s_nreg; i++) {
    Reg &r = s_reg[i];
    if (r.menu == -2 || (r.menu == -1 && protegido(r))) { r.menu = (int8_t)r.def; r.orden = extra++; }
  }

  // armar la tabla de cada menu, ordenada
  uint8_t cuantos[MENUS_N] = {0};
  for (uint8_t m = 0; m < MENUS_N; m++) {
    uint8_t idx[REG_MAX], k = 0;
    for (uint8_t i = 0; i < s_nreg; i++) if (s_reg[i].menu == (int8_t)m) idx[k++] = i;
    for (uint8_t a = 1; a < k; a++) {       // insercion por 'orden'
      const uint8_t v = idx[a];
      int b = a - 1;
      while (b >= 0 && s_reg[idx[b]].orden > s_reg[v].orden) { idx[b + 1] = idx[b]; b--; }
      idx[b + 1] = v;
    }
    for (uint8_t a = 0; a < k && cuantos[m] < REJILLA_MAX_ITEMS; a++)
      s_items[m][cuantos[m]++] = s_reg[idx[a]].item;
  }
  for (uint8_t m = 0; m < MENUS_N; m++) {
    s_cfg[m].items = s_items[m];
    s_cfg[m].n = cuantos[m];
    s_cfg[m].cols = 3;
    s_cfg[m].paginada = true;
    s_cfg[m].atras = MENUS[m].atras;
    s_cfg[m].anterior = MENUS[m].anterior;
  }
  s_slotsVer = slotsVersion();
  s_ver++;
}

static void asegurar()
{
  if (s_nreg == 0 || s_slotsVer != slotsVersion()) reconstruir();
}

// ---------------------------------------------------------------------------
//  Uso desde las pantallas
// ---------------------------------------------------------------------------
void menuEntrar(MenuId m)
{
  asegurar();
  s_abierto = (int8_t)m;
  s_visto[m] = s_ver;
  rejillaEntrar(s_cfg[m]);
}

void menuLoop(MenuId m)
{
  asegurar();
  if (s_visto[m] != s_ver) {            // cambio la distribucion con el menu abierto
    rejillaSalir();
    s_visto[m] = s_ver;
    rejillaEntrar(s_cfg[m]);
  }
  rejillaLoop();
}

void menuSalir() { rejillaSalir(); s_abierto = -1; }

// ---------------------------------------------------------------------------
//  Nombres de los menus Custom
// ---------------------------------------------------------------------------
void menusIniciar()
{
  Preferences p;
  p.begin("menus", true);
  for (uint8_t k = 0; k < 3; k++) {
    char c[4], def[NOMBRE_MAX];
    snprintf(c, sizeof(c), "n%u", (unsigned)k);
    snprintf(def, sizeof(def), "Custom %u", (unsigned)(k + 1));
    strlcpy(s_nombres[k], p.getString(c, def).c_str(), NOMBRE_MAX);
  }
  p.end();
  s_nreg = 0;   // fuerza reconstruir con los nombres
}

// ---------------------------------------------------------------------------
//  Comando 'menu'
//    menu list
//    menu begin / menu add ID CLAVE / menu hide CLAVE / menu commit
//    menu reset
//    menu name N "texto"
// ---------------------------------------------------------------------------
static String s_stg[MENUS_N];
static String s_stgOc;
static uint8_t s_stgCnt[MENUS_N];
static bool    s_stgAbierto = false;

static void ok()               { Serial.println(F("OK")); }
static void err(const char *m) { Serial.print(F("ERR ")); Serial.println(m); }

static void ayuda()
{
  Serial.println(F("  menu list                    menus y botones (MENU / ITEM)"));
  Serial.println(F("  menu begin                   empieza una nueva distribucion"));
  Serial.println(F("  menu add ID CLAVE            pone el boton en el menu ID, al final"));
  Serial.println(F("  menu hide CLAVE              oculta el boton"));
  Serial.println(F("  menu commit                  guarda la distribucion"));
  Serial.println(F("  menu reset                   vuelve todo a como viene de fabrica"));
  Serial.println(F("  menu name N \"texto\"          nombre del menu Custom N (1-3, max 15)"));
  ok();
}

static void escribirEtiqueta(const char *e)
{
  for (; *e; e++) {
    if (*e == '\n') Serial.print(F("\\n"));
    else            Serial.write(*e);
  }
}

static void listar()
{
  asegurar();
  for (uint8_t m = 0; m < MENUS_N; m++) {
    const bool custom = m >= M_CUSTOM1;
    Serial.printf("MENU %u|%s|%s|%u|%u\n", (unsigned)m, MENUS[m].clave,
                  custom ? s_nombres[m - M_CUSTOM1] : MENUS[m].nombre,
                  (unsigned)custom, (unsigned)REJILLA_MAX_ITEMS);
  }
  // en orden: cada menu con sus botones y al final los ocultos
  for (int8_t m = 0; m <= MENUS_N; m++) {
    const int8_t mm = (m == MENUS_N) ? -1 : m;
    uint8_t idx[REG_MAX], k = 0;
    for (uint8_t i = 0; i < s_nreg; i++) if (s_reg[i].menu == mm) idx[k++] = i;
    for (uint8_t a = 1; a < k; a++) {
      const uint8_t v = idx[a];
      int b = a - 1;
      while (b >= 0 && s_reg[idx[b]].orden > s_reg[v].orden) { idx[b + 1] = idx[b]; b--; }
      idx[b + 1] = v;
    }
    for (uint8_t a = 0; a < k; a++) {
      const Reg &r = s_reg[idx[a]];
      Serial.printf("ITEM %s|%d|", r.clave, (int)mm);
      escribirEtiqueta(r.item.etiqueta);
      Serial.printf("|%u\n", (unsigned)r.def);
    }
  }
  ok();
}

static void commit()
{
  Preferences p;
  p.begin("menus", false);
  for (uint8_t m = 0; m < MENUS_N; m++) {
    char k[6];
    snprintf(k, sizeof(k), "l%u", (unsigned)m);
    if (s_stg[m].length()) p.putString(k, s_stg[m]);
    else                   p.remove(k);
  }
  if (s_stgOc.length()) p.putString("oc", s_stgOc);
  else                  p.remove("oc");
  p.end();
  s_stgAbierto = false;
  s_nreg = 0;          // se reconstruye en el siguiente uso
  asegurar();
  ok();
}

bool menusComando(const String &linea)
{
  if (!(linea == "menu" || linea.startsWith("menu "))) return false;

  char buf[200];
  if (linea.length() >= sizeof(buf)) { err("linea demasiado larga"); return true; }
  strlcpy(buf, linea.c_str(), sizeof(buf));
  char *t[5];
  const int n = slotTokenizar(buf, t, 5);
  if (n == -1) { err("comilla sin cerrar"); return true; }
  if (n < 0)   { err("demasiados argumentos"); return true; }

  if (n == 1 || !strcmp(t[1], "help")) { ayuda(); return true; }
  if (!strcmp(t[1], "list")) { listar(); return true; }

  if (!strcmp(t[1], "begin")) {
    for (uint8_t m = 0; m < MENUS_N; m++) { s_stg[m] = ""; s_stgCnt[m] = 0; }
    s_stgOc = "";
    s_stgAbierto = true;
    ok();
    return true;
  }
  if (!strcmp(t[1], "add")) {
    if (!s_stgAbierto)  { err("falta 'menu begin'"); return true; }
    if (n != 4)         { err("uso: menu add ID CLAVE"); return true; }
    asegurar();
    const int id = atoi(t[2]);
    if (id < 0 || id >= MENUS_N) { err("ID de menu invalido"); return true; }
    if (buscar(t[3]) < 0)        { err("boton desconocido"); return true; }
    if (s_stgCnt[id] >= REJILLA_MAX_ITEMS) { err("el menu esta lleno (maximo 28)"); return true; }
    if (s_stg[id].length()) s_stg[id] += ',';
    s_stg[id] += t[3];
    s_stgCnt[id]++;
    ok();
    return true;
  }
  if (!strcmp(t[1], "hide")) {
    if (!s_stgAbierto)  { err("falta 'menu begin'"); return true; }
    if (n != 3)         { err("uso: menu hide CLAVE"); return true; }
    asegurar();
    if (buscar(t[2]) < 0) { err("boton desconocido"); return true; }
    if (s_stgOc.length()) s_stgOc += ',';
    s_stgOc += t[2];
    ok();
    return true;
  }
  if (!strcmp(t[1], "commit")) {
    if (!s_stgAbierto) { err("falta 'menu begin'"); return true; }
    commit();
    return true;
  }
  if (!strcmp(t[1], "reset")) {
    Preferences p;
    p.begin("menus", false);
    for (uint8_t m = 0; m < MENUS_N; m++) {
      char k[6];
      snprintf(k, sizeof(k), "l%u", (unsigned)m);
      p.remove(k);
    }
    p.remove("oc");
    p.end();
    s_nreg = 0;
    asegurar();
    ok();
    return true;
  }
  if (!strcmp(t[1], "name")) {
    if (n != 4) { err("uso: menu name N \"texto\""); return true; }
    const int k = atoi(t[2]);
    if (k < 1 || k > 3) { err("N debe ser 1, 2 o 3"); return true; }
    const char *txt = t[3];
    if (strlen(txt) >= NOMBRE_MAX) { err("nombre de maximo 15 caracteres"); return true; }
    for (const char *c = txt; *c; c++) if (*c < 32 || *c > 126 || *c == '|' || *c == ',') { err("solo ASCII, sin | ni coma"); return true; }
    char c[4];
    snprintf(c, sizeof(c), "n%u", (unsigned)(k - 1));
    Preferences p;
    p.begin("menus", false);
    if (txt[0]) p.putString(c, txt);
    else        p.remove(c);
    p.end();
    if (txt[0]) strlcpy(s_nombres[k - 1], txt, NOMBRE_MAX);
    else        snprintf(s_nombres[k - 1], NOMBRE_MAX, "Custom %d", k);
    s_nreg = 0;
    asegurar();
    ok();
    return true;
  }
  err("subcomando desconocido (escribe 'menu help')");
  return true;
}
