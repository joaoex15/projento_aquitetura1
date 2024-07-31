#include <stdio.h>
#include <stdlib.h>
<<<<<<< HEAD
#include <string.h>
#include <stdint.h>
#include "struct.h"

// Função de conversão hexadecimal para binário
void conversao(struct projeto *proj, int index) {
    int ind_h = 2;  // Começa após o prefixo '0x'
    int ind_b = index * 32;  // Índice para o binário

    while (proj->hexcs[index][ind_h] != '\0') {
        char hex = proj->hexcs[index][ind_h];
        int bin[4] = {0, 0, 0, 0}; // 4 bits

        switch (hex) {
            case '0': memcpy(bin, (int[]){0, 0, 0, 0}, 4 * sizeof(int)); break;
            case '1': memcpy(bin, (int[]){0, 0, 0, 1}, 4 * sizeof(int)); break;
            case '2': memcpy(bin, (int[]){0, 0, 1, 0}, 4 * sizeof(int)); break;
            case '3': memcpy(bin, (int[]){0, 0, 1, 1}, 4 * sizeof(int)); break;
            case '4': memcpy(bin, (int[]){0, 1, 0, 0}, 4 * sizeof(int)); break;
            case '5': memcpy(bin, (int[]){0, 1, 0, 1}, 4 * sizeof(int)); break;
            case '6': memcpy(bin, (int[]){0, 1, 1, 0}, 4 * sizeof(int)); break;
            case '7': memcpy(bin, (int[]){0, 1, 1, 1}, 4 * sizeof(int)); break;
            case '8': memcpy(bin, (int[]){1, 0, 0, 0}, 4 * sizeof(int)); break;
            case '9': memcpy(bin, (int[]){1, 0, 0, 1}, 4 * sizeof(int)); break;
            case 'A': case 'a': memcpy(bin, (int[]){1, 0, 1, 0}, 4 * sizeof(int)); break;
            case 'B': case 'b': memcpy(bin, (int[]){1, 0, 1, 1}, 4 * sizeof(int)); break;
            case 'C': case 'c': memcpy(bin, (int[]){1, 1, 0, 0}, 4 * sizeof(int)); break;
            case 'D': case 'd': memcpy(bin, (int[]){1, 1, 0, 1}, 4 * sizeof(int)); break;
            case 'E': case 'e': memcpy(bin, (int[]){1, 1, 1, 0}, 4 * sizeof(int)); break;
            case 'F': case 'f': memcpy(bin, (int[]){1, 1, 1, 1}, 4 * sizeof(int)); break;
            default:
                printf("Caractere hexadecimal inválido: %c\n", hex);
                return;
        }

        // Copia os bits para o array de saída
        for (int i = 0; i < 4; i++) {
            proj->MEM32[ind_b + i] = bin[i];
        }

        ind_b += 4; // Avança o índice de binário
        ind_h++; // Avança o índice hexadecimal
    }
=======
#include <stdint.h>
#include <string.h>
#include "struct.h"

#define MAX_LINE_LENGTH 12 // Para "0xXXXXXXXX" + '\n' + '\0'

// Função para contar o número de linhas no arquivo
int count_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Não foi possível abrir o arquivo");
        return -1;
    }

    int lines = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        lines++;
    }

    fclose(file);
    return lines;
}

// Função para ler o arquivo e armazenar os valores em HMEM32
int read_hex_to_hmem32(const char *filename, Projeto *proj) {
    proj->line_count = count_lines(filename);
    if (proj->line_count < 0) {
        return -1; // erro ao contar as linhas
    }

    proj->HMEM32 = (uint32_t*)malloc(proj->line_count * sizeof(uint32_t));
    if (proj->HMEM32 == NULL) {
        perror("Erro ao alocar memória");
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Não foi possível abrir o arquivo");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, sizeof(line), file) && index < proj->line_count) {
        // Remove o caractere de nova linha, se presente
        line[strcspn(line, "\n")] = '\0';

        // Verifica se a linha começa com "0x"
        if (strncmp(line, "0x", 2) == 0) {
            uint32_t value;
            if (sscanf(line, "%x", &value) == 1) {
                proj->HMEM32[index] = value;
                index++;
            } else {
                fprintf(stderr, "Erro ao converter linha: %s\n", line);
            }
        }
    }

    fclose(file);
    return 0;
}

// Função para converter HMEM32 para HMEM8
void convert_hmem32_to_hmem8(Projeto *proj) {
    proj->HMEM8 = (uint8_t*)malloc(proj->line_count * 4 * sizeof(uint8_t)); // 4 bytes por valor de 32 bits
    if (proj->HMEM8 == NULL) {
        perror("Erro ao alocar memória para HMEM8");
        exit(1);
    }

    for (int i = 0; i < proj->line_count; ++i) {
        proj->HMEM8[i * 4] = (proj->HMEM32[i] >> 24) & 0xFF; // Byte mais significativo
        proj->HMEM8[i * 4 + 1] = (proj->HMEM32[i] >> 16) & 0xFF;
        proj->HMEM8[i * 4 + 2] = (proj->HMEM32[i] >> 8) & 0xFF;
        proj->HMEM8[i * 4 + 3] = proj->HMEM32[i] & 0xFF; // Byte menos significativo
    }
}

// Função para converter um valor hexadecimal (representado como uint32_t) para um array de inteiros binários
void hex_to_bin32(uint32_t hex, int bin[32]) {
    for (int i = 0; i < 32; i++) {
        bin[31 - i] = (hex >> i) & 1;
    }
}

// Função para converter um valor hexadecimal (representado como uint8_t) para um array de inteiros binários
void hex_to_bin8(uint8_t hex, int bin[8]) {
    for (int i = 0; i < 8; i++) {
        bin[7 - i] = (hex >> i) & 1;
    }
}

// Função para converter HMEM32 e HMEM8 para suas representações binárias
void convert_hex_to_bin(Projeto *proj) {
    // Alocar memória para BMEM32 e BMEM8
    proj->BMEM32 = (int**)malloc(proj->line_count * sizeof(int*));
    for (int i = 0; i < proj->line_count; i++) {
        proj->BMEM32[i] = (int*)malloc(32 * sizeof(int));
        hex_to_bin32(proj->HMEM32[i], proj->BMEM32[i]);
    }

    proj->BMEM8 = (int**)malloc(proj->line_count * 4 * sizeof(int*));
    for (int i = 0; i < proj->line_count * 4; i++) {
        proj->BMEM8[i] = (int*)malloc(8 * sizeof(int));
        hex_to_bin8(proj->HMEM8[i], proj->BMEM8[i]);
    }
}

// Função para imprimir um array de inteiros binários
void print_bin(int *bin, int length) {
    for (int i = 0; i < length; i++) {
        printf("%d", bin[i]);
    }
    printf("\n");
}

// Função para imprimir HMEM32
void print_hmem32(Projeto *proj) {
    printf("\nHMEM32:\n");
    for (int i = 0; i < proj->line_count; ++i) {
        printf("0x%08X: ", proj->HMEM32[i]);
        print_bin(proj->BMEM32[i], 32);
    }
}

// Função para imprimir HMEM8
void print_hmem8(Projeto *proj) {
    printf("\nHMEM8:\n");
    for (int i = 0; i < proj->line_count * 4; ++i) {
        printf("0x%02X: ", proj->HMEM8[i]);
        print_bin(proj->BMEM8[i], 8);
    }
}

// Função que combina todas as operações
void process_file(const char *filename, Projeto *proj) {
    // Ler e converter o arquivo para HMEM32
    if (read_hex_to_hmem32(filename, proj) != 0) {
        exit(1); // erro ao ler o arquivo
    }

    // Converter HMEM32 para HMEM8
    convert_hmem32_to_hmem8(proj);

    // Converter HMEM32 e HMEM8 para binário
    convert_hex_to_bin(proj);

    // Imprimir o conteúdo de HMEM32 e BMEM32
    print_hmem32(proj);

    // Imprimir o conteúdo de HMEM8 e BMEM8
    print_hmem8(proj);

    // Liberar a memória alocada
    free(proj->HMEM32);
    free(proj->HMEM8);
    for (int i = 0; i < proj->line_count; i++) {
        free(proj->BMEM32[i]);
    }
    free(proj->BMEM32);
    for (int i = 0; i < proj->line_count * 4; i++) {
        free(proj->BMEM8[i]);
    }
    free(proj->BMEM8);
}

int main() {
    Projeto proj;
    process_file("/home/rochajs/projento_aquitetura1/1_erro.hex", &proj);
    return 0;
>>>>>>> 72e146e6542ad8ffb6959ac8d48230923ea61b67
}
