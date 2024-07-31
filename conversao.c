#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "struct.h"

// Função de conversão hexadecimal para binário
void conversao(struct projeto *proj, int index) {
    int ind_h = 2;  // Começa após o prefixo '0x'
    int ind_b = index * 32;  // Índice para o binário

    // Inicializa o valor do MEM32 para 0
    uint32_t valor_binario = 0;

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


            for (int i = 0; i < 4; i++) {
            proj->MEM32[ind_b + i] = bin[i];
        }


        // Atualiza o valor do MEM32
        valor_binario = (valor_binario << 4) | (bin[0] << 3) | (bin[1] << 2) | (bin[2] << 1) | bin[3];

        if ((ind_b % 32) == 28) { // Quando 32 bits foram preenchidos
            proj->MEM32[ind_b / 32] = valor_binario;
            valor_binario = 0; // Reseta o valor para o próximo bloco de 32 bits
        }

        ind_b += 4; // Avança o índice de binário
        ind_h++; // Avança o índice hexadecimal
    }

    // Armazena o último valor se não for múltiplo de 32 bits
    if ((ind_b % 32) != 0) {
        proj->MEM32[ind_b / 32] = valor_binario;
    }
}
