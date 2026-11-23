// Compilare: gcc -o s4_basic s4_basic.c -lm
// Rulare:    ./s4_basic 0 3      (cuvant 1 = Marcel, cuvant 2 = schimba)

#include <stdio.h>
#include <math.h>

// cuvintele de tip intrare, indicii corespund cu coloana din matricea B
const char *cuv[6] = {"Marcel", "Alina", "George", "schimba", "repara", "monteaza"};
const char *rasp[3] = {"PLC", "motorul", "HMI"};

// reducere memorie (uitare/estompare pt cuvintele vechi)
const float A = 0.95f;

// etichetele cuvintelor, pereche de doua numer e alese aleator (nu se modifica)
const float B[6][2] = {
    {-0.1298f,  0.2362f},   /* Marcel   */
    {-0.1517f, -0.2219f},   /* Alina    */
    {-0.1691f, -0.1145f},   /* George   */
    {-0.1117f,  0.0357f},   /* schimba  */
    {-0.0394f,  0.1173f},   /* repara   */
    {-0.1704f,  0.1601f}    /* monteaza */
};

// partea antrenata
const float C[3][2] = {
    {-0.2276f,  3.1157f},   /* PLC     */
    {-1.6872f, -2.4847f},   /* motorul */
    {-1.9241f, -0.8352f}    /* HMI     */
};

int main(int argc, char **argv)
{
    int w1 = 0, w2 = 3;          // valori implicite daca nu se primesc argumente in linia de comanda
    float x[2] = {0.0f, 0.0f};   // initializez memoria pe 0 adica goala
    float y[3], e[3], suma = 0.0f;
    int cuvinte[2], i, k, best = 0;

    if (argc == 3) {
        sscanf(argv[1], "%d", &w1);
        sscanf(argv[2], "%d", &w2);
    }
    if (w1 < 0 || w1 > 5 || w2 < 0 || w2 > 5) {
        printf("cuvintele sunt numere intre 0 si 5\n");
        return 1;
    }
    cuvinte[0] = w1;
    cuvinte[1] = w2;

    // citim cuvintele pe rand
    // memorie = A * memorie + eticheta
    for (i = 0; i < 2; i++) {
        for (k = 0; k < 2; k++) {
            x[k] = A * x[k] + B[cuvinte[i]][k];
        }
        printf("dupa \"%s\": memorie = (%.4f, %.4f)\n", cuv[cuvinte[i]], x[0], x[1]);
    }

    // scor pentru fiecare raspuns = randul din C inmultit cu memoria
    for (i = 0; i < 3; i++) {
        y[i] = C[i][0] * x[0] + C[i][1] * x[1];
        printf("scor %-8s = %.3f\n", rasp[i], y[i]);
    }

    // softmax: exp la fiecare scor, apoi impartit la suma -> procente
    for (i = 0; i < 3; i++) {
        e[i] = expf(y[i]);// e la puterea y[i]
        suma += e[i];
    }
    for (i = 0; i < 3; i++) {
        printf("procent %s = %f\n", rasp[i], 100.0f * e[i] / suma);
        if (e[i] > e[best]) {
            best = i;
        }
    }

    // decode: raspunsul este cel cu procentul cel mai mare 
    printf("raspuns: %s\n", rasp[best]);
    return 0;
}