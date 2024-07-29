#include "struct.h"
#include <stdio.h>
#include <string.h>

// Função de conversão hexadecimal para binário
void conversao(struct projeto *proj, int index) {
    int ind_h = 2;  // Começa após o prefixo '0x'
    int ind_b = 0;  // Índice para o binário

    while (proj->hexcs[index][ind_h] != '\0') {
        char hex = proj->hexcs[index][ind_h];
        char bin[5] = {0}; // 4 bits + terminador nulo

        switch (hex) {
            case '0': strcpy(bin, "0000"); break;
            case '1': strcpy(bin, "0001"); break;
            case '2': strcpy(bin, "0010"); break;
            case '3': strcpy(bin, "0011"); break;
            case '4': strcpy(bin, "0100"); break;
            case '5': strcpy(bin, "0101"); break;
            case '6': strcpy(bin, "0110"); break;
            case '7': strcpy(bin, "0111"); break;
            case '8': strcpy(bin, "1000"); break;
            case '9': strcpy(bin, "1001"); break;
            case 'A': case 'a': strcpy(bin, "1010"); break;
            case 'B': case 'b': strcpy(bin, "1011"); break;
            case 'C': case 'c': strcpy(bin, "1100"); break;
            case 'D': case 'd': strcpy(bin, "1101"); break;
            case 'E': case 'e': strcpy(bin, "1110"); break;
            case 'F': case 'f': strcpy(bin, "1111"); break;
            default:
                printf("Caractere hexadecimal inválido: %c\n", hex);
                return;
        }

        // Copia o binário para a string de saída
        strncpy(&proj->bins[index][ind_b], bin, 4);
        ind_b += 4; // Avança o índice de binário
        ind_h++; // Avança o índice hexadecimal
    }

    proj->bins[index][ind_b] = '\0'; // Adiciona terminador nulo
}