#include "struct.h"
#include <stdio.h>
#include <string.h>

// Função de conversão hexadecimal para binário
void conversao(struct projeto *proj, int index) {
    int ind_h = 2;  // Começa após o prefixo '0x'
    int ind_b = 0;  // Índice para o binário

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
            proj->bins[index][ind_b + i] = bin[i];
        }

        ind_b += 4; // Avança o índice de binário
        ind_h++; // Avança o índice hexadecimal
    }

    proj->bins[index][ind_b] = '\0'; // Adiciona terminador nulo
}