#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

struct projeto {
    char hexcs[100][100]; // Array para armazenar as linhas hexadecimais
    uint32_t *MEM32;      // Ponteiro para armazenar os binários convertidos em 32 bits
    uint8_t *MEM8;        // Ponteiro para armazenar os binários convertidos em 8 bits
    char clas[100][100];  // Array para armazenar as classificações
};

// Declaração das funções
void conversao(struct projeto *proj, int index);
void classificacao(struct projeto *proj, int index);
void converter_MEM32_para_MEM8(struct projeto *proj);

#endif // STRUCT_H
