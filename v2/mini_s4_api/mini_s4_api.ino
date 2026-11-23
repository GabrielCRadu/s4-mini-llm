// Mini-S4 ca server Ollama: ESP32 intra pe WiFi si raspunde ca un model Ollama
// O interfata de chat care stie Ollama (Open WebUI, Page Assist, Chatbox...) se conecteaza
// la http://<IP-ul ESP32>:11434 si alege modelul "mini-s4"
// Modelul antrenat e in model.h, generat de antrenare_v2.m (acelasi ca la mini_s4)
// Biblioteca necesara: ArduinoJson (Library Manager)
// Numele si parola WiFi sunt in secrete.h (copiat din secrete_exemplu.h), pe care git il ignora

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "model.h"
#include "secrete.h"   // WIFI_NUME si WIFI_PAROLA

WebServer server(11434);   // 11434 = portul standard Ollama

const int MAX_CUV = 12;   // propozitia are cel mult 12 cuvinte

float x[N];      // memoria
float p[V];      // procentul fiecarui cuvant (sa fie urmatorul)
int prop[MAX_CUV];   // propozitia, ca indici in vocab
int nr = 0;      // cate cuvinte are propozitia

// ---------- modelul (la fel ca in mini_s4) ----------

// cauta cuvantul in vocabular, -1 daca nu exista
int cauta(const char *cuvant) {
  for (int v = 0; v < V; v++) {
    if (strcasecmp(cuvant, vocab[v]) == 0) return v;
  }
  return -1;
}

// citeste un cuvant: memorie = A * memorie + eticheta cuvantului
void citeste(int w) {
  for (int n = 0; n < N; n++) {
    x[n] = A[n] * x[n] + B[w][n];
  }
  prop[nr] = w;
  nr++;
}

// scor = randul din C inmultit cu memoria, apoi softmax -> procente
void prezice() {
  float maxim = -1e30, suma = 0;
  for (int v = 0; v < V; v++) {
    p[v] = 0;
    for (int n = 0; n < N; n++) p[v] += C[v][n] * x[n];
    if (p[v] > maxim) maxim = p[v];
  }
  for (int v = 0; v < V; v++) {
    p[v] = exp(p[v] - maxim);   // scad maximul ca sa nu iasa numere uriase, rezultatul e la fel
    suma += p[v];
  }
  for (int v = 0; v < V; v++) p[v] /= suma;
}

// alege urmatorul cuvant la intamplare, dupa procente (70% -> ales in 70% din cazuri)
int alege() {
  float r = random(10000) / 10000.0;
  for (int v = 0; v < V; v++) {
    r -= p[v];
    if (r < 0) return v;
  }
  return V - 1;
}

// raspunsul la un mesaj: modelul continua propozitia din mesaj
String raspunde(String mesaj) {
  // 1. memoria porneste goala, apoi citim cuvintele pe rand
  for (int n = 0; n < N; n++) x[n] = 0;
  nr = 0;
  char text[200];
  mesaj.toCharArray(text, sizeof(text));
  for (char *cuv = strtok(text, " \r\n"); cuv != NULL && nr < MAX_CUV; cuv = strtok(NULL, " \r\n")) {
    int w = cauta(cuv);
    if (w < 0) {
      String r = String("Nu stiu cuvantul \"") + cuv + "\". Stiu doar:";
      for (int v = 0; v < V; v++) r += String(" ") + vocab[v];
      return r;
    }
    citeste(w);
  }
  if (nr == 0) return "Scrie inceputul unei propozitii, de exemplu: Marcel repara";

  // 2. primele 3 cuvinte care pot urma
  prezice();
  String urmeaza = "(urmatorul cuvant:";
  for (int i = 0; i < 3; i++) {
    int best = 0;
    for (int v = 1; v < V; v++) {
      if (p[v] > p[best]) best = v;
    }
    urmeaza += String(" ") + vocab[best] + " " + int(100 * p[best] + 0.5) + "%";
    p[best] = -1;   // il scot, ca sa-l gasesc pe urmatorul
  }
  urmeaza += ")";

  // 3. modelul termina propozitia singur, pana la "."
  while (strcmp(vocab[prop[nr - 1]], ".") != 0 && nr < MAX_CUV) {
    prezice();
    citeste(alege());
  }

  String r = "";
  for (int i = 0; i < nr; i++) r += String(vocab[prop[i]]) + " ";
  return r + "\n\n" + urmeaza;
}

// ---------- API-ul Ollama ----------

// un raspuns in formatul Ollama: {"model":..., "message":{"role":"assistant","content":text}, "done":gata}
String mesajOllama(String text, bool gata) {
  JsonDocument r;
  r["model"] = "mini-s4";
  r["created_at"] = "2026-01-01T00:00:00Z";
  r["message"]["role"] = "assistant";
  r["message"]["content"] = text;
  r["done"] = gata;
  String json;
  serializeJson(r, json);
  return json;
}

// POST /api/chat: primeste conversatia, raspunde la ultimul mesaj
void chat() {
  JsonDocument cerere;
  deserializeJson(cerere, server.arg("plain"));
  JsonArray mesaje = cerere["messages"];
  String ultimul = mesaje[mesaje.size() - 1]["content"] | "";
  bool stream = cerere["stream"] | true;   // Ollama trimite pe bucati, daca nu se cere altfel

  String text = raspunde(ultimul);
  Serial.println("> " + ultimul);
  Serial.println(text);

  if (stream) {
    // pe bucati (cate o linie): tot textul, apoi o linie goala care spune ca s-a terminat
    server.send(200, "application/x-ndjson", mesajOllama(text, false) + "\n" + mesajOllama("", true) + "\n");
  } else {
    server.send(200, "application/json", mesajOllama(text, true));
  }
}

// GET /api/tags: lista de modele (doar unul)
void modele() {
  server.send(200, "application/json", R"({
    "models": [{
      "name": "mini-s4", "model": "mini-s4", "modified_at": "2026-01-01T00:00:00Z",
      "size": 30000, "digest": "mini-s4",
      "details": {"format": "s4", "family": "s4", "parameter_size": "1.2K", "quantization_level": "F32"}
    }]
  })");
}

void versiune() {
  server.send(200, "application/json", "{\"version\":\"0.9.0\"}");
}

void pornit() {
  server.send(200, "text/plain", "Ollama is running");
}

void altceva() {
  if (server.method() == HTTP_OPTIONS) {
    server.send(204);   // browserul intreaba inainte daca are voie; raspundem ca da
  } else {
    server.send(404, "text/plain", "nu exista");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_NUME, WIFI_PAROLA);
  Serial.print("Ma conectez la WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Gata. In interfata de chat pune adresa Ollama: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":11434");

  server.enableCORS(true);   // ca sa mearga si din interfete care ruleaza in browser
  server.on("/", pornit);
  server.on("/api/version", versiune);
  server.on("/api/tags", modele);
  server.on("/api/chat", HTTP_POST, chat);
  server.onNotFound(altceva);
  server.begin();
}

void loop() {
  server.handleClient();
}
