#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint32_t MEM32[32 * 1024]; // 32 KiB de memória em 32 bits
};

// Atualiza o registrador de status sem a flag de Overflow
void atualizar_SR_sem_OV(uint32_t *R, uint32_t ZN, uint32_t SN, uint32_t CY, uint32_t IV) {
    const uint32_t ZN_MASK = 0b01000000; // Zero flag
    const uint32_t SN_MASK = 0b00001000; // Sign flag
    const uint32_t CY_MASK = 0b00000001; // Carry flag
    const uint32_t IV_MASK = 0b00000010; // Instruction Valid flag

    uint32_t sr = R[31];
    sr &= ~(ZN_MASK | SN_MASK | CY_MASK | IV_MASK);

    if (ZN) sr |= ZN_MASK; // Setar Zero Flag
    if (SN) sr |= SN_MASK; // Setar Sign Flag
    if (CY) sr |= CY_MASK; // Setar Carry Flag
    if (IV) sr |= IV_MASK; // Setar Instruction Valid Flag
    
    R[31] = sr;
}

// Atualiza o registrador de status sem a flag de Carry
void atualizar_SR_sem_CY(uint32_t *R, uint32_t ZN, uint32_t SN, uint32_t OV, uint32_t IV) {
    const uint32_t ZN_MASK = 0b01000000; // Zero flag
    const uint32_t SN_MASK = 0b00001000; // Sign flag
    const uint32_t OV_MASK = 0b00001000; // Overflow flag
    const uint32_t IV_MASK = 0b00000010; // Instruction Valid flag

    uint32_t sr = R[31];
    sr &= ~(ZN_MASK | SN_MASK | OV_MASK | IV_MASK);

    if (ZN) sr |= ZN_MASK; // Setar Zero Flag
    if (SN) sr |= SN_MASK; // Setar Sign Flag
    if (OV) sr |= OV_MASK; // Setar Overflow Flag
    if (IV) sr |= IV_MASK; // Setar Instruction Valid Flag
    
    R[31] = sr;
}

int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    int index = 0;
    uint32_t R[32] = {0};

    R[29] = 0; // Inicializa o PC (R29) com o endereço inicial

    hex = fopen("entrada.txt", "r");
    if (hex == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    while (fgets(linha, sizeof(linha), hex) != NULL) {
        linha[strcspn(linha, "\n")] = '\0';
        proj.MEM32[index] = (uint32_t)strtol(linha, NULL, 16);
        index++;
    }
    fclose(hex);

    uint8_t executa = 1;
    while (executa) {
        char instrucao[30] = {0};
        uint8_t z = 0, x = 0, y = 0,i=0;
        int32_t l = 0;
        uint32_t pc = 0;
        uint8_t ZN = 0, SN = 0, OV = 0, IV = 0, CY = 0;

        R[30] = proj.MEM32[R[29] >> 2];

        uint8_t opcode = (R[30] >> 26) & 0x3F;

        switch (opcode) {
            case 0b000000: // mov
                z = (R[30] >> 21) & 0x1F;
                l = R[30] & 0x1FFFFF;
                R[z] = l;
                sprintf(instrucao, "mov R%u,%u", z, l);
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;

            case 0b000001: // movs
                z = (R[30] >> 21) & 0x1F;
                l = R[30] & 0x1FFFFF;
                if (l & (1 << 20)) {
                    l |= 0xFFE00000;
                }
                R[z] = l;
                sprintf(instrucao, "movs R%u,%d", z, l);
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;

            case 0b000010: { // add
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;
                y = (R[30] >> 11) & 0x1F;

                uint64_t resultado = (uint64_t)R[x] + (uint64_t)R[y];
                R[z] = (uint32_t)resultado;

                ZN = (R[z] == 0) ? 1 : 0;
                SN = ((int32_t)R[z] < 0) ? 1 : 0;
                CY = (resultado > 0xFFFFFFFF) ? 1 : 0;
                OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31)) && ((R[z] & (1 << 31)) == (R[y] & (1 << 31)))) ? 1 : 0);

                atualizar_SR_sem_OV(R, ZN, SN, CY, IV);

                sprintf(instrucao, "add R%u,R%u,R%u", z, x, y);
                printf("0x%08X:\t%-25s\tR%u=R%u+R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

            case 0b000011: { // sub
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;
                y = (R[30] >> 11) & 0x1F;

                int64_t resultado = (int64_t)R[x] - (int64_t)R[y];
                R[z] = (uint32_t)resultado;

                ZN = (R[z] == 0) ? 1 : 0;
                SN = ((int32_t)R[z] < 0) ? 1 : 0;
                CY = (resultado < 0) ? 1 : 0;
                OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31)) && ((R[z] & (1 << 31)) != (R[y] & (1 << 31)))) ? 1 : 0);

                if (SN) {
                    atualizar_SR_sem_CY(R, ZN, SN, OV, IV);
                } else {
                    atualizar_SR_sem_OV(R, ZN, SN, CY, IV);
                }

                sprintf(instrucao, "sub R%u,R%u,R%u", z, x, y);
                printf("0x%08X:\t%-25s\tR%u=R%u-R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

            case 0b000100: { // div
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;
                y = (R[30] >> 11) & 0x1F;

                if (R[y] == 0) {
                    IV = 1; // Divisão por zero
                } else {
                    R[z] = R[x] / R[y];
                    IV = 0;
                }

                ZN = (R[z] == 0) ? 1 : 0;
                SN = ((int32_t)R[z] < 0) ? 1 : 0;
                CY = 0; // A flag de carry não é afetada por divisão
                OV = 0; // A flag de overflow não é afetada por divisão

                atualizar_SR_sem_OV(R, ZN, SN, CY, IV);

                sprintf(instrucao, "div R%u,R%u,R%u", z, x, y);
                printf("0x%08X:\t%-25s\tR%u=R%u/R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

            case 0b000101: { // divs
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;
                y = (R[30] >> 11) & 0x1F;

                if (R[y] == 0) {
                    IV = 1; // Divisão por zero
                } else {
                    R[z] = (int32_t)R[x] / (int32_t)R[y];
                    IV = 0;
                }

                ZN = (R[z] == 0) ? 1 : 0;
                SN = ((int32_t)R[z] < 0) ? 1 : 0;
                CY = 0; // A flag de carry não é afetada por divisão
                OV = 0; // A flag de overflow não é afetada por divisão

                atualizar_SR_sem_OV(R, ZN, SN, CY, IV);

                sprintf(instrucao, "divs R%u,R%u,R%u", z, x, y);
                printf("0x%08X:\t%-25s\tR%u=R%u/R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

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

            default:
                printf("Instrução desconhecida.\n");
                IV = 1; // Instrução inválida
                atualizar_SR_sem_OV(R, ZN, SN, CY, IV);
                break;
        }

        R[29] += 4; // Incrementa o PC
    

    return 0;
}
