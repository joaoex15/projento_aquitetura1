#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint32_t MEM32[32 * 1024]; // 32 KiB de memória em 32 bits
};

void atualizar_SR(uint32_t *R, uint32_t ZN, uint32_t SN, uint32_t OV, uint32_t CY, uint32_t IV) {
    R[31] = ZN | SN | OV | CY | IV;
}

int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    FILE *saida; // Arquivo para saída
    int index = 0;
    uint32_t R[32] = {0};

    // Inicializa o PC (R29) com o endereço inicial
    R[29] = 0;

    hex = fopen("entrada.txt", "r");
    if (hex == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        return 1;
    }

    // Lê cada linha do arquivo até o final
    while (fgets(linha, sizeof(linha), hex) != NULL) {
        // Remove o caractere de nova linha se presente
        linha[strcspn(linha, "\n")] = '\0';

        // Converte a linha hexadecimal para uint32_t e armazena em MEM32
        proj.MEM32[index] = (uint32_t)strtol(linha, NULL, 16);
        index++;
    }

    fclose(hex);

    // Abre o arquivo de saída
    saida = fopen("saida.txt", "w");
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        return 1;
    }

    uint8_t executa = 1;
    // Enquanto executa for verdadeiro
    while (executa) {
        // Cadeia de caracteres da instrução
        char instrucao[30] = {0};
        // Declarando operandos
        uint8_t z = 0, x = 0, y = 0;
        int32_t l = 0;
        uint32_t pc = 0;
        uint8_t ZN = 0, ZD = 0, SN = 0, OV = 0, IV = 0, CY = 0;

        // Carregando a instrução de 32 bits (4 bytes) da memória indexada pelo PC (R29) no registrador IR (R30)
        R[30] = proj.MEM32[R[29] >> 2];

        // Obtendo o código da operação (6 bits mais significativos)
        uint8_t opcode = (R[30] & (0b111111 << 26)) >> 26;

        // Decodificando a instrução buscada na memória
        switch (opcode) {
            // mov
            case 0b000000:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                l = R[30] & 0x1FFFFF;
                // Execução do comportamento
                R[z] = l;
                // Atualizando SR
                // Formatação da instrução
                sprintf(instrucao, "mov R%u,%u", z, l);
                // Formatação de saída em arquivo
                fprintf(saida, "0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;

            // movs
            case 0b000001:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                l = (R[30] & 0x1FFFFF);
                // Extensão de sinal
                if (l & (1 << 20)) {
                    l |= 0xFFE00000; // Extende o sinal para os 21 bits
                }
                // Execução do comportamento
                R[z] = l;
                // Atualizando SR
                // Formatação da instrução
                sprintf(instrucao, "movs R%u,%d", z, l);
                // Formatação de saída em arquivo
                fprintf(saida, "0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;

            // add
            case 0b000010: { // Opcode para add
                // Obtendo operandos
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;
                y = (R[30] >> 11) & 0x1F;

                // Execução da adição
                uint64_t resultado = (uint64_t)R[x] + (uint64_t)R[y];
                R[z] = (uint32_t)resultado;

                // Atualizando flags
                ZN = (R[z] == 0) ? 0x1 : 0x0; // Zero flag
                SN = ((int32_t)R[z] < 0) ? 0x1 : 0x0; // Sign flag
                OV = (((R[x] & (1 << 31)) == (R[y] & (1 << 31)) && (R[z] & (1 << 31)) != (R[x] & (1 << 31))) ? 0x1 : 0x0); // Overflow flag
                CY = (resultado > 0xFFFFFFFF) ? 0x1 : 0x0; // Carry flag
                IV = 0x0; // Presumindo que a operação é válida

                // Atualizando o registrador de status
                atualizar_SR(R, ZN, SN, OV, CY, IV);

                // Formatação da instrução
                sprintf(instrucao, "add r%u,r%u,r%u", z, x, y);
                fprintf(saida, "0x%08X:\t%-25s\tR%u=R%u+R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

            // sub
            case 0b000011: { // Opcode para sub
                // Obtendo operandos
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;
                y = (R[30] >> 11) & 0x1F;

                // Execução da subtração
                uint64_t resultado = (uint64_t)R[x] - (uint64_t)R[y];
                R[z] = (uint32_t)resultado;

                // Atualizando flags
                ZN = (R[z] == 0) ? 0x1 : 0x0; // Zero flag
                SN = ((int32_t)R[z] < 0) ? 0x1 : 0x0; // Sign flag
                OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31)) && (R[z] & (1 << 31)) != (R[x] & (1 << 31))) ? 0x1 : 0x0); // Overflow flag
                CY = (resultado > 0xFFFFFFFF) ? 0x1 : 0x0; // Carry flag
                IV = 0x0; // Presumindo que a operação é válida

                // Atualizando o registrador de status
                atualizar_SR(R, ZN, SN, OV, CY, IV);

                // Formatação da instrução
                sprintf(instrucao, "sub r%u,r%u,r%u", z, x, y);
                fprintf(saida, "0x%08X:\t%-25s\tR%u=R%u-R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

            // l8
            case 0b011000:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                x = (R[30] & (0b11111 << 16)) >> 16;
                l = R[30] & 0xFFFF;
                // Execução do comportamento com MEM32 (cálculo do índice da palavra e seleção do byte big-endian)
                R[z] = ((uint8_t*)(&proj.MEM32[(R[x] + l) >> 2]))[3 - ((R[x] + l) % 4)];
                // Atualizando SR
                ZN = (R[z] == 0) ? 0x1 : 0x0;
                SN = ((int32_t)R[z] < 0) ? 0x1 : 0x0;
                OV = 0x0;
                CY = 0x0;
                IV = 0x0;
                atualizar_SR(R, ZN, SN, OV, CY, IV);
                // Formatação da instrução
                sprintf(instrucao, "l8 R%u,R%u,%d", z, x, l);
                // Formatação de saída em arquivo
                fprintf(saida, "0x%08X:\t%-25s\tR%u=0x%02X,SR=0x%08X\n", R[29], instrucao, z, R[z], R[31]);
                break;

            // s8
            case 0b011001:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                x = (R[30] & (0b11111 << 16)) >> 16;
                l = R[30] & 0xFFFF;
                // Execução do comportamento com MEM32 (cálculo do índice da palavra e seleção do byte big-endian)
                ((uint8_t*)(&proj.MEM32[(R[x] + l) >> 2]))[3 - ((R[x] + l) % 4)] = R[z] & 0xFF;
                // Atualizando SR
                ZN = ((uint8_t)R[z] == 0) ? 0x1 : 0x0;
                SN = ((int32_t)R[z] < 0) ? 0x1 : 0x0;
                OV = 0x0;
                CY = 0x0;
                IV = 0x0;
                atualizar_SR(R, ZN, SN, OV, CY, IV);
                // Formatação da instrução
                sprintf(instrucao, "s8 R%u,R%u,%d", z, x, l);
                // Formatação de saída em arquivo
                fprintf(saida, "0x%08X:\t%-25s\tMEM32[0x%08X] = 0x%02X,SR=0x%08X\n", R[29], instrucao, R[x] + l, R[z] & 0xFF, R[31]);
                break;

            // Instrução não reconhecida
            default:
                fprintf(saida, "0x%08X:\tInstrução desconhecida\n", R[29]);
                executa = 0; // Termina a execução
                break;
        }

        // Atualiza o PC (R29) para a próxima instrução
        R[29] += 4;
    }

    // Fecha o arquivo de saída
    fclose(saida);

    return 0;
}
