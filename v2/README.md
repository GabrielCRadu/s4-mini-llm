# Mini-S4 v2

Acelasi model ca in v1 (`x = A*x + B*u`, `y = C*x`, softmax), dar mai mare: 39 de cuvinte, 32 de propozitii, memorie de 32 de numere. Modelul continua propozitii: "Marcel repara" -> "PLC-ul din hala .".

Ce e diferit fata de v1:

- `A` are alta uitare pe fiecare componenta (de la 0.3 la 0.95), deci conteaza si ordinea cuvintelor.
- `C` se antreneaza cu gradient descent pe eroarea cross-entropy (3000 de pasi), ca la LLM-urile mari, in loc de `pinv`. Procentele ies corecte: dupa "Marcel", `schimba` are 25%, adica exact cat apare in date.
- Antrenarea foloseste fiecare inceput de propozitie: din "Alina repara motorul defect ." ies 4 exemple.

## Fisiere

    antrenare_v2.m          propozitiile, antrenarea, teste, scrie model.h
    mini_s4/                ESP32 prin Serial Monitor
    mini_s4_api/            ESP32 ca server Ollama, pentru interfete de chat
    */model.h               matricile antrenate (generate, nu se scriu de mana)

## Rulare

1. MATLAB, in folderul `v2`: `antrenare_v2`. Afiseaza teste si rescrie `model.h` in ambele foldere.
2. Arduino IDE: deschizi `mini_s4/mini_s4.ino`, placa "ESP32 Dev Module", Upload.
3. Serial Monitor la 115200, "New Line". Scrii `Marcel repara`:

        urmeaza: PLC-ul 97%   cablul 2%   de 0%
        propozitie: Marcel repara PLC-ul din hala .

Pentru alte propozitii, le schimbi in `antrenare_v2.m` si rulezi din nou.

## Ca server Ollama

In `mini_s4_api.ino` pui numele si parola WiFi (2.4 GHz), Upload. Serial Monitor arata adresa, de exemplu `http://192.168.1.50:11434`. Pe aceasta adresa o pui in interfata de chat la "Ollama URL" si alegi modelul `mini-s4`.

Test din PowerShell:

    curl.exe http://192.168.1.50:11434/api/chat -d '{\"messages\":[{\"role\":\"user\",\"content\":\"Marcel repara\"}],\"stream\":false}'
