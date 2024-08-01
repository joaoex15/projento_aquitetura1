#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint32_t MEM32[32 * 1024]; // 32 KiB de memória em 32 bits
    uint8_t MEM8[32 * 1024];   // 32 KiB de memória em 8 bits
};

int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    int index = 0;
    uint32_t R[32] = {0};

    // Inicializa o PC (R29) com o endereço inicial
    R[29] = 0;

    hex = fopen("1_erro.hex", "r");
    if (hex == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Lê cada linha do arquivo até o final
    while (fgets(linha, sizeof(linha), hex) != NULL) {
        // Remove o caractere de nova linha se presente
        linha[strcspn(linha, "\n")] = '\0';

        // Converte a linha hexadecimal para uint32_t e armazena em MEM32
        proj.MEM32[index] = (uint32_t)strtol(linha, NULL, 16);

        // Armazena o valor também em MEM8 para acesso byte a byte
        for (int i = 0; i < 4; i++) {
            proj.MEM8[index * 4 + i] = (proj.MEM32[index] >> (24 - i * 8)) & 0xFF;
        }

        index++;
    }

    fclose(hex);

    uint8_t executa = 1;
    // Enquanto executa for verdadeiro
    while (executa) {
        // Cadeia de caracteres da instrução
        char instrucao[30] = {0};
        // Declarando operandos
        uint8_t z = 0, x = 0, i = 0;
        uint32_t pc = 0, xyl = 0;
        // Carregando a instrução de 32 bits (4 bytes) da memória indexada pelo PC (R29) no registrador IR (R28)
        // É feita a leitura redundante com MEM8 e MEM32 para mostrar formas equivalentes de acesso
        // Se X (MEM8) for igual a Y (MEM32), então X e Y são iguais a X | Y (redundância)
        R[30] = ((proj.MEM8[R[29] + 0] << 24) | (proj.MEM8[R[29] + 1] << 16) | (proj.MEM8[R[29] + 2] << 8) | (proj.MEM8[R[29] + 3] << 0)) | proj.MEM32[R[29] >> 2];
        // Obtendo o código da operação (6 bits mais significativos)
        uint8_t opcode = (R[30] & (0b111111 << 26)) >> 26;
        // Decodificando a instrução buscada na memória
        switch (opcode) {
            // mov
            case 0b000000:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                xyl = R[30] & 0x1FFFFF;
                // Execução do comportamento
                R[z] = xyl;
                // Formatação da instrução
                sprintf(instrucao, "mov R%u,%u", z, xyl);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, xyl);
                break;
            // l8
            case 0b011000:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                x = (R[30] & (0b11111 << 16)) >> 16;
                i = R[30] & 0xFFFF;
                // Execução do comportamento com MEM8 e MEM32 (cálculo do índice da palavra e seleção do byte big-endian)
                R[z] = proj.MEM8[R[x] + i] | (((uint8_t*)(&proj.MEM32[(R[x] + i) >> 2]))[3 - ((R[x] + i) % 4)]);
                // Formatação da instrução
                sprintf(instrucao, "l8 R%u,[R%u%s%i]", z, x, (i >= 0) ? ("+") : (""), i);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=MEM[0x%08X]=0x%02X\n", R[29], instrucao, z, R[x] + i, R[z]);
                break;
            // l32
            case 0b011010:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                x = (R[30] & (0b11111 << 16)) >> 16;
                i = R[30] & 0xFFFF;
                // Execução do comportamento com MEM8 e MEM32
                R[z] = ((proj.MEM8[((R[x] + i) << 2) + 0] << 24) | (proj.MEM8[((R[x] + i) << 2) + 1] << 16) | (proj.MEM8[((R[x] + i) << 2) + 2] << 8) | (proj.MEM8[((R[x] + i) << 2) + 3] << 0)) | proj.MEM32[R[x] + i];
                // Formatação da instrução
                sprintf(instrucao, "l32 R%u,[R%u%s%i]", z, x, (i >= 0) ? ("+") : (""), i);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=MEM[0x%08X]=0x%08X\n", R[29], instrucao, z, (R[x] + i) << 2, R[z]);
                break;
            // bun
            case 0b110111:
                // Armazenando o PC antigo
                pc = R[29];
                // Execução do comportamento
                R[29] = R[29] + ((R[30] & 0x3FFFFFF) << 2);
                // Formatação da instrução
                sprintf(instrucao, "bun %i", R[30] & 0x3FFFFFF);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tPC=0x%08X\n", pc, instrucao, R[29] + 4);
                break;
            // int
            case 0b111111:
                // Parar a execução
                executa = 0;
                // Formatação da instrução
                sprintf(instrucao, "int 0");
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tCR=0x00000000,PC=0x00000000\n", R[29], instrucao);
                break;
            // Instrução desconhecida
            default:
                // Exibindo mensagem de erro
                sprintf(instrucao, "Comando desconhecido");
                printf("0x%08X:\t%-25s\n", R[29], instrucao);
                break;
        }
        // PC = PC + 4 (próxima instrução)
        R[29] = R[29] + 4;
    }

    return 0;
}
