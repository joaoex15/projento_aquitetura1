#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint32_t MEM32[32 * 1024]; // 32 KiB de memória em 32 bits
    uint8_t MEM8[32 * 1024 * 4]; // 32 KiB de memória em 8 bits (expandido para suportar l8 e l32)
};

// Atualiza o registrador de status sem a flag de Overflow
void atualizar_SR(uint32_t *R, uint32_t ZN,uint32_t ZD, uint32_t SN,uint32_t OV, uint32_t IV ,uint32_t CY) {
    const uint32_t ZN_MASK = 0b00000000000000000000000001000000; // Zero flag
    const uint32_t ZD_MASK = 0b00000000000000000000000000100000;
    const uint32_t SN_MASK = 0b00000000000000000000000000010000; // Sign flag
    const uint32_t OV_MASK = 0b00000000000000000000000000001000; //ov
    const uint32_t IV_MASK = 0b00000000000000000000000000000100; // Instruction Valid flag
    const uint32_t CY_MASK = 0b00000000000000000000000000000001; // Carry flag

    uint32_t sr = R[31];
    sr &= ~(ZN_MASK |ZD_MASK| SN_MASK |OV_MASK|IV_MASK| CY_MASK );

    if (ZN) sr |= ZN_MASK; // Setar Zero Flag
    if (ZD) sr |= ZD_MASK;
    if (SN) sr |= SN_MASK; // Setar Sign Flag
    if (OV_MASK) sr |= OV_MASK; // Setar Zero Flag
    if (IV) sr |= IV_MASK; // Setar Instruction Valid Flag
    if (CY) sr |= CY_MASK; // Setar Carry Flag

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
        uint8_t z = 0, x = 0, y = 0, i = 0;
        int32_t l = 0;
        uint32_t pc = 0;
        uint8_t ZN = 0,ZD=0, SN = 0, OV = 0, IV = 0, CY = 0;

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

                atualizar_SR(R, ZN, SN, CY, IV);

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

      case 0b000100: { // Opcode base para todas as instruções
    uint32_t z = (R[30] >> 21) & 0x1F;
    uint32_t x = (R[30] >> 16) & 0x1F;
    uint32_t y = (R[30] >> 11) & 0x1F;
    uint32_t l = R[30] & 0x1F;
    uint32_t func = (R[30] >> 5) & 0x7;

    if (func == 0b000) { // mul - Multiplicação sem sinal
        uint64_t result = (uint64_t)R[x] * (uint64_t)R[y];
        R[l] = (uint32_t)(result >> 32); // Parte alta
        R[z] = (uint32_t)result;         // Parte baixa

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        CY = (result != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN, 0, CY, 0, 0);

        sprintf(instrucao, "mul R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u*R%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, l, z, x, y, R[l], R[z], R[31]);
    } else if (func == 0b001) { // sll - Deslocamento lógico para a esquerda
        uint64_t combined = ((uint64_t)R[z] << 32) | R[y];
        uint64_t result = combined << (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        CY = (R[z] != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN, 0, CY, 0, 0);

        sprintf(instrucao, "sll R%u,R%u,R%u", z, x, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u:R%u<<%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, z, y, l + 1, R[z], R[x], R[31]);
    } else if (func == 0b010) { // muls - Multiplicação com sinal
        int64_t result = (int64_t)R[x] * (int64_t)R[y];
        R[l] = (uint32_t)(result >> 32); // Parte alta
        R[z] = (uint32_t)result;         // Parte baixa

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        OV = (result != (int64_t)(int32_t)R[z]) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN, OV, 0, 0, 0);

        sprintf(instrucao, "muls R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u*R%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, l, z, x, y, R[l], R[z], R[31]);
    } else if (func == 0b011) { // sla - Deslocamento aritmético para a esquerda
        int64_t combined = ((int64_t)R[z] << 32) | R[y];
        int64_t result = combined << (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        OV = (result != (int32_t)R[z]) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN, OV, 0, 0, 0);

        sprintf(instrucao, "sla R%u,R%u,R%u", z, x, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u:R%u<<%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, z, y, l + 1, R[z], R[x], R[31]);
    } else if (func == 0b100) { // div - Divisão sem sinal
        if (R[y] != 0) {
            R[z] = R[x] / R[y];
            R[l] = R[x] % R[y];
        } else {
            R[z] = 0; // Divisão por zero
            R[l] = 0;
        }

        ZN = (R[z] == 0) ? 0b01000000 : 0x00000000;
        ZD = (R[y] == 0) ? 0b00010000 : 0x00000000;
        CY = (R[l] != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN, 0, CY, ZD, 0);

        sprintf(instrucao, "div R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u=R%u/R%u=0x%08X, R%u=R%u%%R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], l, x, y, R[l], R[31]);
    } else if (func == 0b101) { // srl - Deslocamento lógico para a direita
        uint64_t combined = ((uint64_t)R[z] << 32) | R[y];
        uint64_t result = combined >> (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        CY = ((result & 0x01) != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN, 0, CY, 0, 0);

        sprintf(instrucao, "srl R%u,R%u,R%u", z, x, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u:R%u>>%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, z, y, l + 1, R[z], R[x], R[31]);
    } else if (func == 0b110) { // divs - Divisão com sinal
        if (R[y] != 0) {
            int32_t div = (int32_t)R[x] / (int32_t)R[y];
            int32_t mod = (int32_t)R[x] % (int32_t)R[y];
            R[z] = (uint32_t)div;
            R[l] = (uint32_t)mod;
        } else {
            R[z] = 0; // Divisão por zero
            R[l] = 0;
        }

        ZN = (R[z] == 0) ? 0b01000000 : 0x00000000;
        ZD = (R[y] == 0) ? 0b00010000 : 0x00000000;
        OV = (R[z] != (int32_t)R[z]) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN, OV, 0, ZD, 0);

        sprintf(instrucao, "divs R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u=R%u/R%u=0x%08X, R%u=R%u%%R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], l, x, y, R[l], R[31]);
    } else if (func == 0b111) { // sra - Deslocamento aritmético para a direita
        int64_t combined = ((int64_t)R[z] << 32) | R[y];
        int64_t result = combined >> (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        OV = ((result & 0x80000000) != 0) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN, OV, 0, 0, 0);

        sprintf(instrucao, "sra R%u,R%u,R%u", z, x, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u:R%u>>%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, z, y, l + 1, R[z], R[x], R[31]);
    }
    break;
}


            case 0b000101: { // l8
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;

                if (R[x] < 0 || R[x] >= sizeof(proj.MEM8)) {
                    IV = 1;
                    atualizar_SR_sem_OV(R, ZN, SN, CY, IV);
                    sprintf(instrucao, "l8 R%u, R%u (Endereço fora dos limites)", z, x);
                } else {
                    R[z] = proj.MEM8[R[x]];
                    ZN = (R[z] == 0) ? 1 : 0;
                    SN = ((int32_t)R[z] < 0) ? 1 : 0;
                    CY = 0;
                    OV = 0;

                    atualizar_SR_sem_CY(R, ZN, SN, OV, IV);

                    sprintf(instrucao, "l8 R%u,R%u", z, x);
                }
                printf("0x%08X:\t%-25s\tR%u=0x%02X, SR=0x%08X\n", R[29], instrucao, z, R[z], R[31]);
                break;
            }

            case 0b000110: { // l32
                z = (R[30] >> 21) & 0x1F;
                x = (R[30] >> 16) & 0x1F;

                if (R[x] < 0 || R[x] >= sizeof(proj.MEM32)) {
                    IV = 1;
                    atualizar_SR_sem_OV(R, ZN, SN, CY, IV);
                    sprintf(instrucao, "l32 R%u,R%u (Endereço fora dos limites)", z, x);
                } else {
                    R[z] = proj.MEM32[R[x] >> 2];
                    ZN = (R[z] == 0) ? 1 : 0;
                    SN = ((int32_t)R[z] < 0) ? 1 : 0;
                    CY = 0;
                    OV = 0;

                    atualizar_SR_sem_CY(R, ZN, SN, OV, IV);

                    sprintf(instrucao, "l32 R%u,R%u", z, x);
                }
                printf("0x%08X:\t%-25s\tR%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, R[z], R[31]);
                break;
            }

            case 0b000111: { // bun
                x = (R[30] >> 21) & 0x1F;
                int32_t offset = (R[30] & 0x1FFFFF) << 2; // Endereço de salto, deslocamento multiplicado por 4

                if (offset < 0 || (uint32_t)offset >= sizeof(proj.MEM32) * 4) {
                    IV = 1;
                    atualizar_SR_sem_OV(R, ZN, SN, CY, IV);
                    sprintf(instrucao, "bun R%u, endereço fora dos limites", x);
                } else {
                    R[29] = offset; // Atualiza o PC com o endereço de salto
                    sprintf(instrucao, "bun R%u", x);
                }
                printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
                break;
            }

            case 0b001000: { // int
                x = (R[30] >> 21) & 0x1F;
                R[31] |= (1 << x); // Define a flag de interrupção correspondente

                sprintf(instrucao, "int R%u", x);
                printf("0x%08X:\t%-25s\tSR=0x%08X\n", R[29], instrucao, R[31]);
                break;
            }

            default:
                IV = 1;
                atualizar_SR_sem_OV(R, ZN, SN, CY, IV);
                sprintf(instrucao, "instrução inválida");
                printf("0x%08X:\t%-25s\n", R[29], instrucao);
                break;
        }

        R[29] += 4; // Incrementa o PC para a próxima instrução
        if (R[29] >= sizeof(proj.MEM32) * 4) {
            executa = 0;
        }
    }

    return 0;
}
