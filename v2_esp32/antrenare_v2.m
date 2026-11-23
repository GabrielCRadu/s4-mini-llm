% antrenare_v2: mini-S4 mai mare, care continua propozitii
% rulare: din folderul v2_esp32, scrie "antrenare_v2" in Command Window
% la final scrie mini_s4/model.h, folosit de programul de pe ESP32
clear; clc;

% propozitiile de antrenare (fara diacritice, fiecare se termina cu " .")
prop = {
    'Marcel schimba PLC-ul defect .'
    'Marcel repara PLC-ul din hala .'
    'Marcel verifica PLC-ul si cablul .'
    'Marcel monteaza un PLC nou .'
    'Marcel porneste PLC-ul dimineata .'
    'Marcel schimba releul de la PLC .'
    'Marcel programeaza PLC-ul azi .'
    'Marcel opreste PLC-ul seara .'
    'Alina repara motorul defect .'
    'Alina schimba motorul din hala .'
    'Alina verifica motorul si pompa .'
    'Alina monteaza un motor nou .'
    'Alina porneste motorul dimineata .'
    'Alina curata ventilatorul de la motor .'
    'Alina opreste motorul seara .'
    'Alina repara pompa din hala .'
    'George monteaza HMI-ul nou .'
    'George verifica HMI-ul din hala .'
    'George repara HMI-ul defect .'
    'George schimba ecranul de la HMI .'
    'George porneste HMI-ul dimineata .'
    'George programeaza HMI-ul azi .'
    'George curata ecranul si panoul .'
    'George opreste HMI-ul seara .'
    'Ioana verifica senzorul din hala .'
    'Ioana schimba senzorul defect .'
    'Ioana monteaza un senzor nou .'
    'Ioana curata senzorul si cablul .'
    'Ioana repara cablul de la senzor .'
    'Ioana calibreaza senzorul azi .'
    'Ioana porneste pompa dimineata .'
    'Ioana opreste pompa seara .'
};

% vocabularul = toate cuvintele diferite, in ordinea in care apar
vocab = unique(strsplit(strjoin(prop', ' ')), 'stable');
V = numel(vocab);
N = 32;                              % cat de mare e memoria

A = linspace(0.3, 0.95, N)';         % uitarea: fiecare componenta a memoriei uita cu alta viteza
B = 0.5 * sin((1:N)' * (1:V));       % eticheta fiecarui cuvant = o coloana din B (fixa, nu se antreneaza)

% H: memoria dupa fiecare inceput de propozitie
% T: cuvantul care urmeaza (1 pe pozitia lui, 0 in rest)
H = [];
T = [];
for s = 1:numel(prop)
    cuv = strsplit(prop{s});
    for k = 1:numel(cuv)-1
        H(:, end+1) = memorie(cuv(1:k), vocab, A, B);
        T(:, end+1) = double(strcmp(vocab, cuv{k+1}))';
    end
end

% antrenare: C porneste de la 0 si se corecteaza de 3000 de ori
% la fiecare pas: vede cat de mult greseste procentele si muta C putin in directia buna
C = zeros(V, N);
for pas = 1:3000
    P = softmax_s4(C*H);                    % procentele prezise pentru toate exemplele
    C = C - 2 * (P - T) * H' / size(H, 2);  % corectia
end

% test: ce cuvant urmeaza?
teste = {'Marcel repara', 'George repara', 'Ioana repara', 'Alina porneste motorul'};
for i = 1:numel(teste)
    p = softmax_s4(C * memorie(strsplit(teste{i}), vocab, A, B));
    [pSort, ord] = sort(p, 'descend');
    fprintf('%-24s -> %s %.0f%%, %s %.0f%%, %s %.0f%%\n', teste{i}, ...
        vocab{ord(1)}, 100*pSort(1), vocab{ord(2)}, 100*pSort(2), vocab{ord(3)}, 100*pSort(3));
end

% test: modelul termina singur propozitii, alegand la intamplare dupa procente
fprintf('\n');
nume = {'Marcel', 'Alina', 'George', 'Ioana'};
for i = 1:numel(nume)
    cuv = nume(i);
    while ~strcmp(cuv{end}, '.') && numel(cuv) < 12
        p = softmax_s4(C * memorie(cuv, vocab, A, B));
        w = find(rand < cumsum(p), 1);       % 70% -> ales in 70% din cazuri
        cuv{end+1} = vocab{w};
    end
    disp(strjoin(cuv, ' '));
end

% scrie modelul pentru ESP32
f = fopen(fullfile('mini_s4', 'model.h'), 'w');
fprintf(f, '// generat de antrenare_v2.m, nu modifica de mana\n\n');
fprintf(f, 'const int N = %d;   // marimea memoriei\n', N);
fprintf(f, 'const int V = %d;   // numarul de cuvinte\n\n', V);
fprintf(f, 'const char *vocab[V] = {');
fprintf(f, '"%s", ', vocab{:});
fprintf(f, '};\n\n');
fprintf(f, '// uitarea pe fiecare componenta a memoriei\n');
fprintf(f, 'const float A[N] = {');
fprintf(f, '%.6ff, ', A);
fprintf(f, '};\n\n');
fprintf(f, '// B[cuvant][n]: eticheta fiecarui cuvant\n');
fprintf(f, 'const float B[V][N] = {\n');
fprintf(f, ['    {' repmat('%.6ff, ', 1, N) '},\n'], B);
fprintf(f, '};\n\n');
fprintf(f, '// C[cuvant][n]: partea antrenata\n');
fprintf(f, 'const float C[V][N] = {\n');
fprintf(f, ['    {' repmat('%.6ff, ', 1, N) '},\n'], C');
fprintf(f, '};\n');
fclose(f);
copyfile(fullfile('mini_s4', 'model.h'), fullfile('mini_s4_api', 'model.h'));   % acelasi model si pentru API
fprintf('\nam scris model.h in mini_s4 si mini_s4_api (%d cuvinte, memorie %d)\n', V, N);

% ---- functii ----
function x = memorie(cuv, vocab, A, B)
    % citeste cuvintele pe rand: memorie = A * memorie + eticheta cuvantului
    x = zeros(size(A));
    for k = 1:numel(cuv)
        x = A.*x + B(:, strcmp(vocab, cuv{k}));
    end
end

function p = softmax_s4(y)
    e = exp(y - max(y));   % scad maximul ca sa nu iasa numere uriase, rezultatul e la fel
    p = e ./ sum(e);       % pe fiecare coloana, procentele fac 100%
end
