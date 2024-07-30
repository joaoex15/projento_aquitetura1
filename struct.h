#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h> // Inclua isso para garantir que uint32_t e uint8_t sejam reconhecidos

// Estrutura do projeto
typedef struct {
    char hexcs[100][20];  // Array para armazenar strings hexadecimais
    int bins[100][32];    // 32 bits binários (inteiros)
    char clas[100][100];  // classificação de formatos
    uint32_t *HMEM32;     // Antigo MEM32
    uint8_t *HMEM8;       // Antigo MEM8
    int **BMEM32;         // Novo array para armazenar valores binários de HMEM32
    int **BMEM8;          // Novo array para armazenar valores binários de HMEM8
    int line_count;
} Projeto;

// Declaração da função de conversão
void process_file(const char *filename, Projeto *proj);

#endif // STRUCT_H





