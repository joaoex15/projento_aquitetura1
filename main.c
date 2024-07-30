#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "struct.h"
#include "conversao.c"

void classificacao(struct projeto *proj, int index) {
    int bin_opcode[6] = {0};  // Para armazenar os 6 bits do opcode
    int bin_opcodeT[32] = {0}; // Para armazenar os primeiros 32 bits
    int all_zero = 1; // Flag para verificar se todos os 32 bits são zero

    // Copia os primeiros 6 bits
    for (int i = 0; i < 6; i++) {
        bin_opcode[i] = proj->bins[index][i];
    }

    // Copia os primeiros 32 bits e verifica se todos são zero
    for (int i = 0; i < 32; i++) {
        bin_opcodeT[i] = proj->bins[index][i];
        if (proj->bins[index][i] != 0) {
            all_zero = 0;
        }
    }

    // Verifica se todos os 32 bits são zero
    if (all_zero) {
        snprintf(proj->clas[index], sizeof(proj->clas[index]), "operação ociosa");
    } else {
        if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 0, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "mov R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 0, 1}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "movs R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 1, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "add R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 1, 1}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "sub R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 0, 0, 1, 0, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "mul R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 1, 1}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "cmp R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 1, 1, 0, 0, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "l8 R[] = MEM8[]");
        } else if (memcmp(bin_opcode, (int[]){0, 1, 1, 0, 1, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "l32 R[] = MEM32[]");
        } else if (memcmp(bin_opcode, (int[]){1, 1, 0, 1, 1, 1}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "bun");
        } else if (memcmp(bin_opcode, (int[]){1, 1, 1, 1, 1, 1}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "int");
        } else {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "Instrução desconhecida: %d%d%d%d%d%d",
    bin_opcode[0], bin_opcode[1], bin_opcode[2], bin_opcode[3], bin_opcode[4], bin_opcode[5]);
        }
    }

    // Exemplo de saída para verificação
    printf("Opcode: ");
    for (int i = 0; i < 6; i++) {
        printf("%d", bin_opcode[i]);
    }
    printf(", Classificação: %s\n", proj->clas[index]);
}

int main() {
    FILE *hex;
    char linha[100];  // Array para armazenar cada linha lida
    struct projeto proj;  // Declaração de uma instância da struct projeto
    int index = 0;  // Índice para armazenar comandos no array hexcs

    hex = fopen("/home/joao/Documentos/programas/ufs/projentos-arquitetura/1_erro.hex", "r");
    if (hex == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Lê cada linha do arquivo até o final
    while (fgets(linha, sizeof(linha), hex) != NULL) {
        // Remove o caractere de nova linha se presente
        linha[strcspn(linha, "\n")] = '\0';

        // Armazena a linha lida diretamente no array hexcs da struct projeto
        snprintf(proj.hexcs[index], sizeof(proj.hexcs[0]), "%s", linha);

        // Chama a função de conversão para a linha atual
        conversao(&proj, index);

        // Chama a função de categorização para processar e classificar
        classificacao(&proj, index);

        index++;
    }

    fclose(hex);
    return 0;
}