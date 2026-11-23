# Mini-S4 v2 - ESP32

The same model as v1 (`x = A*x + B*u`, `y = C*x`, softmax), but bigger: 39 words, 32 sentences and a memory of 32 numbers. It runs on an ESP32 and continues sentences: "Marcel repara" -> "PLC-ul din hala .".

What's different from v1:

- `A` fades each part of the memory at a different rate (from 0.3 to 0.95), so the model can also remember the order of words.
- `C` is trained with gradient descent on cross-entropy (3000 steps), like large LLMs, instead of `pinv`. The percentages come out right: after "Marcel", `schimba` gets 25%, exactly how often it follows "Marcel" in the data.
- Every beginning of a sentence is a training example: "Alina repara motorul defect ." gives 4 examples.

## Files

    antrenare_v2.m             sentences, training and tests; writes model.h
    mini_s4/                   ESP32 sketch: type a sentence in the Serial Monitor
    mini_s4_api/               ESP32 sketch: Ollama-compatible server for chat apps
        secrete_exemplu.h      Wi-Fi name and password template
    */model.h                  trained matrices (generated, don't edit by hand)

## What you need

- An ESP32 board, for example an ESP32-WROOM dev board. Choose "ESP32 Dev Module" in the Arduino IDE.
- Arduino IDE with the ESP32 boards package (Boards Manager: "esp32" by Espressif).
- The ArduinoJson library (Library Manager), needed only for `mini_s4_api`.
- MATLAB, needed only if you want to change the training sentences. The trained `model.h` files are already included.

## Running in the Serial Monitor

1. In the Arduino IDE, open `mini_s4/mini_s4.ino`, pick the board and port, and click Upload. If it gets stuck on `Connecting...`, hold the BOOT button.
2. Open the Serial Monitor at 115200 baud with "New Line", then type `Marcel repara`:

        urmeaza: PLC-ul 97%   cablul 2%   de 0%
        propozitie: Marcel repara PLC-ul din hala .

`urmeaza` shows the 3 most likely next words. `propozitie` is the sentence the model finished by itself. It picks each word at random according to the percentages, so the same start can give different sentences.

The model only knows the 39 words from its training sentences, without diacritics. An unknown word gets the reply `Nu stiu cuvantul` ("I don't know the word").

## As an Ollama server (chat apps)

1. In the `mini_s4_api` folder, copy `secrete_exemplu.h` to `secrete.h` and put your Wi-Fi name and password in it. The network must be 2.4 GHz. Git ignores `secrete.h`, so the password never reaches GitHub.
2. Upload `mini_s4_api/mini_s4_api.ino`. The Serial Monitor prints the board's address, for example `http://192.168.1.50:11434`.
3. In a chat app that supports Ollama (for example the Page Assist extension for Chrome), set the Ollama URL to that address and choose the model `mini-s4`.

Test from PowerShell:

    curl.exe http://192.168.1.50:11434/api/chat -H "Content-Type: application/json" -d '{"messages":[{"role":"user","content":"Marcel repara"}],"stream":false}'

The server implements `/api/tags` (model list), `/api/chat` (with and without streaming) and `/api/version`.

## Changing the sentences

Edit the list at the top of `antrenare_v2.m`. Separate words with spaces, end each sentence with ` .`, and don't use diacritics. Then, in the `v2_esp32` folder in MATLAB, run `antrenare_v2`. It prints the tests and rewrites `model.h` in both sketch folders. The ESP32 code doesn't change: upload it again and it uses the new model.
