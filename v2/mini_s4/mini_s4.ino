// Mini-S4 pe ESP32: scrii inceputul unei propozitii, modelul o continua
// Modelul antrenat (vocab, A, B, C) e in model.h, generat de antrenare_v2.m
// Serial Monitor: 115200 baud, "New Line". Exemplu: Marcel repara

#include "model.h"

const int MAX_CUV = 12;   // propozitia are cel mult 12 cuvinte

float x[N];      // memoria
float p[V];      // procentul fiecarui cuvant (sa fie urmatorul)
int prop[MAX_CUV];   // propozitia, ca indici in vocab
int nr = 0;      // cate cuvinte are propozitia

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

void setup() {
  Serial.begin(115200);
  Serial.println("Scrie inceputul unei propozitii, de exemplu: Marcel repara");
}

void loop() {
  if (Serial.available() == 0) return;   // asteptam un rand de text

  char text[100];
  int lungime = Serial.readBytesUntil('\n', text, sizeof(text) - 1);
  text[lungime] = '\0';

  // 1. memoria porneste goala, apoi citim cuvintele pe rand
  for (int n = 0; n < N; n++) x[n] = 0;
  nr = 0;
  for (char *cuv = strtok(text, " \r"); cuv != NULL && nr < MAX_CUV; cuv = strtok(NULL, " \r")) {
    int w = cauta(cuv);
    if (w < 0) {
      Serial.printf("Nu stiu cuvantul \"%s\"\n", cuv);
      return;
    }
    citeste(w);
  }
  if (nr == 0) return;

  // 2. primele 3 cuvinte care pot urma
  prezice();
  Serial.print("urmeaza: ");
  for (int i = 0; i < 3; i++) {
    int best = 0;
    for (int v = 1; v < V; v++) {
      if (p[v] > p[best]) best = v;
    }
    Serial.printf("%s %.0f%%   ", vocab[best], 100 * p[best]);
    p[best] = -1;   // il scot, ca sa-l gasesc pe urmatorul
  }
  Serial.println();

  // 3. modelul termina propozitia singur, cuvant cu cuvant, pana la "."
  while (strcmp(vocab[prop[nr - 1]], ".") != 0 && nr < MAX_CUV) {
    prezice();
    citeste(alege());
  }

  Serial.print("propozitie: ");
  for (int i = 0; i < nr; i++) {
    Serial.print(vocab[prop[i]]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println();
}
