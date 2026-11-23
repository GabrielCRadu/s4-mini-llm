% antrenare: calculeaza partea antrenata (matricea C) pentru mini-S4
% rulare: scrie "antrenare" in Command Window, din folderul cu fisierul
clear; clc;
rng(1);   % seed fix, ca B sa iasa la fel de fiecare data

vocab = {'Marcel','Alina','George','schimba','repara','monteaza', ...
         'modul','PLC','motorul','HMI'};
V = numel(vocab);
N = 2;    % cat de mare e memoria

A = 0.95*eye(N);      % uitare
B = randn(N,V)*0.2;   % etichetele cuvintelor, random o singura data

% propozitii de antrenare si raspunsul lor
% Marcel schimba -> PLC | Alina repara -> motorul | George monteaza -> HMI
prompts = [1 4; 2 5; 3 6];
tinte   = [8 9 10];
nProp   = size(prompts,1);

% H: memoria dupa ultimul cuvant, cate o coloana pe propozitie
H = zeros(N, nProp);
for s = 1:nProp
    x = zeros(N,1);
    for k = 1:size(prompts,2)
        u = cuvant_enc(prompts(s,k), V);
        x = A*x + B*u;
    end
    H(:,s) = x;
end
disp('H ='); disp(H);

% T: raspunsul corect pentru fiecare propozitie
T = eye(nProp);

% C: partea antrenata
C = T*pinv(H);
disp('C ='); disp(C);

% test pe propozitiile invatate
for s = 1:nProp
    x = zeros(N,1);
    for k = 1:size(prompts,2)
        x = A*x + B*cuvant_enc(prompts(s,k), V);
    end
    y = C*x;
    p = softmax_s4(y);
    idCuvant = decode_s4(p, tinte);
    fprintf('%s %s -> %s (p = %.2f)\n', vocab{prompts(s,1)}, ...
        vocab{prompts(s,2)}, vocab{idCuvant}, max(p));
end

% valorile de mai sus, in formatul din s4_basic.c
fprintf('\nB (primele 6 cuvinte):\n');
fprintf('    {%.4ff, %.4ff},\n', B(:,1:6));
fprintf('C:\n');
fprintf('    {%.4ff, %.4ff},\n', C');

% ---- functii ----
function u = cuvant_enc(idx, V)
    % vector cu 1 pe pozitia cuvantului, 0 in rest
    u = zeros(V,1);
    if idx >= 1 && idx <= V
        u(idx) = 1;
    end
end

function p = softmax_s4(y)
    e = exp(y - max(y));   % scad maximul ca sa nu iasa numere uriase, rezultatul e la fel
    p = e / sum(e);
end

function idCuvant = decode_s4(p, tinte)
    % clasa cu procentul cel mai mare -> cuvantul ei
    [~, clasa] = max(p);
    idCuvant = tinte(clasa);
end
