# Mini-S4

Un LLM in miniatura bazat pe S4 (structured state space), pornit de la exemplul simplificat din cursul MM-ISI.
Modelul primeste doua cuvinte si ghiceste urmatorul: "Marcel schimba" -> PLC.

## Cum functioneaza

Totul e un sistem cu stare:

    x(k+1) = A*x(k) + B*u(k)      memoria
    y      = C*x                  scorurile

1. Fiecare cuvant are o eticheta fixa de 2 numere (o coloana din `B`), trasa la intamplare o singura data.
2. Memoria `x` porneste goala. La fiecare cuvant se micsoreaza la 95% (`A = 0.95`) si primeste eticheta cuvantului.
3. Scorul fiecarui raspuns = un rand din `C` inmultit cu memoria.
4. Softmax transforma scorurile in procente, iar cel mai mare procent da raspunsul.

Doar `C` este antrenat: `C = T*pinv(H)`, unde `H` are pe coloane memoria finala a fiecarei propozitii de antrenare, iar `T` spune raspunsul corect pentru fiecare.

Vocabular: Marcel, Alina, George, schimba, repara, monteaza, modul, PLC, motorul, HMI.
Propozitii de antrenare: `Marcel schimba -> PLC`, `Alina repara -> motorul`, `George monteaza -> HMI`.

## Structura

    matlab/antrenare.m     calculeaza H si C (antrenarea) si testeaza cele 3 propozitii
    c/s4_basic.c           rularea modelului in C, cu valorile antrenate puse in cod

## Rulare

MATLAB: in folderul `matlab`, in Command Window:

    antrenare

Afiseaza `H`, `C`, rezultatul pe cele 3 propozitii si valorile `B` si `C` in formatul din `s4_basic.c`.

C (orice calculator cu gcc, de exemplu Raspberry Pi):

    gcc -o s4_basic s4_basic.c -lm
    ./s4_basic 0 3

Argumentele sunt indicii cuvintelor: 0 Marcel, 1 Alina, 2 George, 3 schimba, 4 repara, 5 monteaza.
