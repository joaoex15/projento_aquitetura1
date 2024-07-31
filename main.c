#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "conversao.c"
// Função de categorização
void classificacao(struct projeto *proj, int index) {
    char bin_opcode[7] = {0}; // Para armazenar o opcode em formato binário (6 bits + terminador nulo)
    char  bin_opcodeT[33] = {0};
    char bin_x[5];
    char bin_y[5];
    char bin_z[5];
    uint32_t R[32] = { 0 };

<<<<<<< HEAD
    // Copia os primeiros 6 bits
    for (int i = 0; i < 6; i++) {
        bin_opcode[i] = proj->MEM32[index * 32 + i];
    }

    // Copia os primeiros 32 bits e verifica se todos são zero
    for (int i = 0; i < 32; i++) {
        bin_opcodeT[i] = proj->MEM32[index * 32 + i];
        if (proj->MEM32[index * 32 + i] != 0) {
            all_zero = 0;
        }
    }
=======
    // Obtém os primeiros 6 bits da string binária
    strncpy(bin_opcode, proj->bins[index], 6);
    

    // Obtém os primeiros 32 bits da string binária
    strncpy(bin_opcodeT, proj->bins[index], 32);
>>>>>>> 72e146e6542ad8ffb6959ac8d48230923ea61b67

    // Verifica se todos os 32 bits são zero
    if (strncmp(bin_opcodeT, "00000000000000000000000000000000", 32) == 0) {
        snprintf(proj->clas[index], sizeof(proj->clas[index]), "operação ociosa");
    } else {
<<<<<<< HEAD
        if (memcmp(bin_opcode, (int[]){0, 0, 0, 0, 0, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "mov R[] = R[]");
        } else if (memcmp(bin_opcode, (int[]){0, 0, 1, 0, 0, 0}, 6 * sizeof(int)) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "soma R[] = R[] + R[]");
        } else {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "operação desconhecida");
        }
    }
=======
        if (strncmp(bin_opcode, "000000", 6) == 0) {
        
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "mov R[] = R[]" );
        }
        else if (strncmp(bin_opcode, "000001", 6) == 0) {
    
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "movs R[] = R[]" );
        }


    else if (strncmp(bin_opcode, "000010", 6) == 0) {
        
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "add R[] = R[]" );
            strncpy(bin_x,proj->bins[index] +  11, 5);
            strncpy(bin_y,proj->bins[index] +  16, 5);
            
        }
    else if (strncmp(bin_opcode, "000011", 6) == 0) {
    
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "Sub R[] = R[]" );
        }

    else if (strncmp(bin_opcode, "000100", 6) == 0) {
        // A varias operações similares com msm 6 digitos principais
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "mul R[] = R[]" );
        }
    else if (strncmp(bin_opcode, "000011", 6) == 0) {
    
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "Cmp R[] = R[]" );
        }




        else if (strncmp(bin_opcode, "011000", 6) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "l8 R[] = MEM8[]" );
        } else if (strncmp(bin_opcode, "011010", 6) == 0) {

            snprintf(proj->clas[index], sizeof(proj->clas[index]), "l32 R[] = MEM32[]" );
        } else if (strncmp(bin_opcode, "110111", 6) == 0) {
    
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "bun " );
        } else if (strncmp(bin_opcode, "111111", 6) == 0) {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "int");
        } else {
            snprintf(proj->clas[index], sizeof(proj->clas[index]), "Instrução desconhecida: %s", bin_opcode);
        }
    }

    // Exemplo de saída para verificação
    printf("Opcode: %s, Classificação: %s\n", bin_opcode, proj->clas[index]);
>>>>>>> 72e146e6542ad8ffb6959ac8d48230923ea61b67
}

int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    int index = 0;

    // Aloca memória para MEM32
    proj.MEM32 = (uint32_t *)malloc(100 * 32 * sizeof(uint32_t));
    if (proj.MEM32 == NULL) {
        perror("Erro ao alocar memória para MEM32");
        return 1;
    }

    hex = fopen("/home/joao/Documentos/programas/ufs/projentos-arquitetura/1_erro.hex", "r");
    if (hex == NULL) {
        perror("Erro ao abrir o arquivo");
        free(proj.MEM32); // Libera a memória alocada em caso de erro
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

    // Impressão dos resultados
    printf("Hexadecimal e Binário:\n");
    for (int i = 0; i < index; i++) {
        printf("Linha %d: %s\n", i, proj.hexcs[i]);

        printf("Binário: ");
        for (int j = 0; j < 32; j++) {
            printf("%d", proj.MEM32[i * 32 + j]);
        }
        printf("\nClassificação: %s\n\n", proj.clas[i]);
    }

    free(proj.MEM32); // Libera a memória alocada para MEM32
    return 0;
}
