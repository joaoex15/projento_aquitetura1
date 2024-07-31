#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "conversao.c"

int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    int index = 0;

    // Aloca memória para MEM32 e MEM8
    proj.MEM32 = (uint32_t *)malloc(100 * sizeof(uint32_t)); // Ajuste o tamanho conforme necessário
    proj.MEM8 = (uint8_t *)malloc(100 * 4 * sizeof(uint8_t)); // Ajuste o tamanho conforme necessário (4 bytes por MEM32)
    if (proj.MEM32 == NULL || proj.MEM8 == NULL) {
        perror("Erro ao alocar memória para MEM32 ou MEM8");
        free(proj.MEM32); // Libera a memória alocada em caso de erro
        free(proj.MEM8);  // Libera a memória alocada em caso de erro
        return 1;
    }

    hex = fopen("/home/rochajs/projento_aquitetura1/1_erro.hex", "r");
    if (hex == NULL) {
        perror("Erro ao abrir o arquivo");
        free(proj.MEM32); // Libera a memória alocada em caso de erro
        free(proj.MEM8);  // Libera a memória alocada em caso de erro
        return 1;
    }

    // Lê cada linha do arquivo até o final
    while (fgets(linha, sizeof(linha), hex) != NULL) {
        // Remove o caractere de nova linha se presente
        linha[strcspn(linha, "\n")] = '\0';

        // Armazena a linha lida diretamente no array hexcs da struct projeto
        snprintf(proj.hexcs[index], sizeof(proj.hexcs[0]), "%s", linha);

        // Exibe a linha lida
        printf("Linha lida: %s\n", proj.hexcs[index]);

        // Converte a linha hexadecimal para MEM32 e MEM8
        sscanf(proj.hexcs[index], "%x", &proj.MEM32[index]);
        for (int i = 0; i < 4; i++) {
            proj.MEM8[index * 4 + i] = (proj.MEM32[index] >> (24 - i * 8)) & 0xFF;
        }

        // Lógica de classificação
        int bin_opcode[6] = {0}; // Para armazenar os 6 bits do opcode
        int bin_opcodeT[32] = {0}; // Para armazenar os primeiros 32 bits
        int all_zero = 1; // Flag para verificar se todos os 32 bits são zero

        // Copia os primeiros 6 bits
        for (int i = 0; i < 6; i++) {
            bin_opcode[i] = (proj.MEM32[index] >> (31 - i)) & 1;
        }

        // Copia os primeiros 32 bits e verifica se todos são zero
        for (int i = 0; i < 32; i++) {
            bin_opcodeT[i] = (proj.MEM32[index] >> (31 - i)) & 1;
            if (bin_opcodeT[i] != 0) {
                all_zero = 0;
            }
        }

        // Verifica se todos os 32 bits são zero
        if (all_zero) {
            snprintf(proj.clas[index], sizeof(proj.clas[index]), "operação ociosa");
        } else {
            if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 0, 0}, 6 * sizeof(int)) == 0) {
                snprintf(proj.clas[index], sizeof(proj.clas[index]), "mov R[] = R[]");
            } else if (memcmp(bin_opcode, (int[]){0, 0, 1, 0, 0, 0}, 6 * sizeof(int)) == 0) {
                snprintf(proj.clas[index], sizeof(proj.clas[index]), "soma R[] = R[] + R[]");
            } else {
                snprintf(proj.clas[index], sizeof(proj.clas[index]), "operação desconhecida");
            }
        }

        // Exibe a classificação para a linha atual
        printf("Classificação: %s\n", proj.clas[index]);

        index++;
    }

    fclose(hex);

    // Libera a memória alocada
    free(proj.MEM32);
    free(proj.MEM8);
    
    return 0;
}
