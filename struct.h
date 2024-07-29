#ifndef CONVERSAO_H
#define CONVERSAO_H

// Estrutura do projeto
struct projeto {
    char hexcs[100][100];
    char bins[100][33];  // 32 bits + terminador nulo
    char cate[100];      // categorização que é para as operações
    char clas[100][100]; // classificação de formatos
};

// Declaração da função de conversão
void conversao(struct projeto *proj, int index);

#endif // CONVERSAO_H
