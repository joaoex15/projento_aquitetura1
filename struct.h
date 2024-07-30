#ifndef CONVERSAO_H
#define CONVERSAO_H

// Estrutura do projeto
    struct projeto {
    char hexcs[100][20];  // Array para armazenar strings hexadecimais
    int bins[100][32];    // 32 bits binários (inteiros)

    char clas[100][100]; // classificação de formatos
};

// Declaração da função de conversão
void conversao(struct projeto *proj, int index);

#endif // CONVERSAO_H
