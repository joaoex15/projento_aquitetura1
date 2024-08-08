#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint8_t* MEM8 ;
	uint32_t* MEM32 ;
};

// Atualiza o registrador de status sem a flag de Overflow
void atualizar_SR(uint32_t *R, uint32_t ZN, uint32_t ZD, uint32_t SN, uint32_t OV, uint32_t IV, uint32_t CY) {
    // Máscaras para cada flag no registrador de status
    const uint32_t ZN_MASK = 0b00000000000000000000000001000000; // Zero flag (bit 6)
    const uint32_t ZD_MASK = 0b00000000000000000000000000100000; // Zero Detect flag (bit 5)
    const uint32_t SN_MASK = 0b00000000000000000000000000010000; // Sign flag (bit 4)
    const uint32_t OV_MASK = 0b00000000000000000000000000001000; // Overflow flag (bit 3)
    const uint32_t IV_MASK = 0b00000000000000000000000000000100; // Instruction Valid flag (bit 2)
    const uint32_t CY_MASK = 0b00000000000000000000000000000001; // Carry flag (bit 0)

    // Obter o valor atual do registrador de status
    uint32_t sr = R[31];
    
    // Limpar os bits dos flags antes de atualizar
    sr &= ~(ZN_MASK | ZD_MASK | SN_MASK | OV_MASK | IV_MASK | CY_MASK);
    
    // Configurar os bits dos flags com base nos valores fornecidos
    if (ZN) sr |= ZN_MASK;
    if (ZD) sr |= ZD_MASK;
    if (SN) sr |= SN_MASK;
    if (OV) sr |= OV_MASK;
    if (IV) sr |= IV_MASK;
    if (CY) sr |= CY_MASK;

    // Atualizar o registrador de status
    R[31] = sr;
}



int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    int index = 0;
    uint32_t R[32] = {0};
    proj.MEM8 = (uint8_t*)calloc(32 * 1024, sizeof(uint8_t));
    proj.MEM32 = (uint32_t*)calloc(32 * 1024, sizeof(uint32_t));

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
        size_t tamanho_mem8 = 32 * 1024; // 32 KiB
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
                if (x<0&&y<0)
                {
                                    OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31)) && ((R[z] & (1 << 31)) == (R[y] & (1 << 31)))) ? 1 : 0);
                }
                else
                {
                            CY = (resultado > 0xFFFFFFFF) ? 1 : 0;
                }
                
    
                atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

                sprintf(instrucao, "add R%u,R%u,R%u", z, x, y);
                printf("0x%08X:\t%-25s\tR%u=R%u+R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                break;
            }

 

 case 0b000011: { // sub
    z = (R[30] >> 21) & 0x1F; // Registrador destino
    x = (R[30] >> 16) & 0x1F; // Registrador fonte 1
    y = (R[30] >> 11) & 0x1F; // Registrador fonte 2

    int64_t resultado = (int64_t)R[x] - (int64_t)R[y]; // Subtração com sinal
    R[z] = (uint32_t)resultado; // Resultado da subtração

    // Atualização dos flags
    ZN = (R[z] == 0) ? 1 : 0; // Zero Flag (ZN)
    SN = (R[z] & (1 << 31)) ? 1 : 0; // Sign Flag (SN)

    // Overflow Flag (OV) para operações com sinal
    OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31))) && ((R[z] & (1 << 31)) != (R[x] & (1 << 31)))) ? 1 : 0;

    // Carry Flag (CY) para operações sem sinal
    // Em uma subtração, se houver um "empréstimo" (borrow), o carry flag deve ser ajustado.
    CY = (resultado < 0) ? 1 : 0;

    // Atualiza o registrador de status
    atualizar_SR(R, ZN, 0, SN, OV, 0, CY);

    // Impressão do resultado
    sprintf(instrucao, "sub R%u,R%u,R%u", z, x, y);
    printf("0x%08X:\t%-25s\tR%u=R%u-R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
    break;
}





    case 0b000100:{
        uint32_t func =(R[30]>>8)&0x7;
        if (func == 0b000) { // mul - Multiplicação sem sinal
        uint64_t result = (uint64_t)R[x] * (uint64_t)R[y];
        R[l] = (uint32_t)(result >> 32); // Parte alta
        R[z] = (uint32_t)result;         // Parte baixa

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        CY = (result != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

        sprintf(instrucao, "mul R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u*R%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, l, z, x, y, R[l], R[z], R[31]);
    } else if (func == 0b001) { // sll - Deslocamento lógico para a esquerda
        uint64_t combined = ((uint64_t)R[z] << 32) | R[y];
        uint64_t result = combined << (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        CY = (R[z] != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

        sprintf(instrucao, "sll R%u,R%u,R%u", z, x, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u:R%u<<%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, z, y, l + 1, R[z], R[x], R[31]);
    } else if (func == 0b010) { // muls - Multiplicação com sinal
        int64_t result = (int64_t)R[x] * (int64_t)R[y];
        R[l] = (uint32_t)(result >> 32); // Parte alta
        R[z] = (uint32_t)result;         // Parte baixa

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        OV = (result != (int64_t)(int32_t)R[z]) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

        sprintf(instrucao, "muls R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u*R%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, l, z, x, y, R[l], R[z], R[31]);
    } else if (func == 0b011) { // sla - Deslocamento aritmético para a esquerda
        int64_t combined = ((int64_t)R[z] << 32) | R[y];
        int64_t result = combined << (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        OV = (result != (int32_t)R[z]) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

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

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

        sprintf(instrucao, "div R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u=R%u/R%u=0x%08X, R%u=R%u%%R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], l, x, y, R[l], R[31]);
    } else if (func == 0b101) { // srl - Deslocamento lógico para a direita
        uint64_t combined = ((uint64_t)R[z] << 32) | R[y];
        uint64_t result = combined >> (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        CY = ((result & 0x01) != 0) ? 0b00000001 : 0x00000000;

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

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

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

        sprintf(instrucao, "divs R%u,R%u,R%u", l, z, y);
        printf("0x%08X:\t%-25s\tR%u=R%u/R%u=0x%08X, R%u=R%u%%R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], l, x, y, R[l], R[31]);
    } else if (func == 0b111) { // sra - Deslocamento aritmético para a direita
        int64_t combined = ((int64_t)R[z] << 32) | R[y];
        int64_t result = combined >> (l + 1);
        R[z] = (uint32_t)(result >> 32);
        R[x] = (uint32_t)result;

        ZN = (result == 0) ? 0b01000000 : 0x00000000;
        OV = ((result & 0x80000000) != 0) ? 0b00001000 : 0x00000000;

        atualizar_SR(R, ZN,ZD, SN,OV, IV,CY);

        sprintf(instrucao, "sra R%u,R%u,R%u", z, x, y);
        printf("0x%08X:\t%-25s\tR%u:R%u=R%u:R%u>>%u=0x%08X:0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, z, y, l + 1, R[z], R[x], R[31]);
    }
    break; } 


    










case 0b011000: { // Opcode para l8
                uint32_t z = (R[30] >> 21) & 0x1F; // Registrador destino (5 bits)
                uint32_t x = (R[30] >> 16) & 0x1F; // Registrador base (5 bits)
                int16_t i = R[30] & 0xFFFF; // Deslocamento (16 bits)

                // Calcula o endereço de memória
                uint32_t addr = R[x] + i;

                if (addr < (32 * 1024)) { // Verifica se o endereço está dentro dos limites (32 * 1024 bytes)
                    R[z] = proj.MEM8[addr]; // Carrega o valor de 8 bits da memória para o registrador

                    // Atualizando as flags ZN e SN
                    ZN = (R[z] == 0);
                    SN = (R[z] & 0x80) != 0;

                    atualizar_SR(R, ZN, ZD, SN, OV, IV, CY);
                    sprintf(instrucao, "l8 R%u,[R%u+0x%04X]", z, x, i);
                    printf("0x%08X:\t%-25s\tR%u=0x%02X, SR=0x%08X\n", pc, instrucao, z, R[z], R[31]);
                }
                break;
            }

            case 0b011010: { // l32
                z = (R[30] >> 21) & 0b11111;
                x = (R[30] >> 16) & 0b11111;
                l = R[30] & 0xFFFF;

                uint32_t endereco = (R[x] + l) >> 2;

                if (endereco < (32 * 1024)) {
                    R[z] = proj.MEM32[endereco];

                    // Atualizando as flags ZN e SN
                    ZN = (R[z] == 0);
                    SN = (R[z] & 0x80000000) != 0;

                    atualizar_SR(R, ZN, ZD, SN, OV, IV, CY);
                    sprintf(instrucao, "l32 R%u,[R%u%s%i]", z, x, (l >= 0) ? ("+") : (""), l);
                    printf("0x%08X:\t%-25s\tR%u=MEM[0x%08X]=0x%08X, SR=0x%08X\n", R[29], instrucao, z, (R[x] + l), R[z], R[31]);
                }
                break;
            }
            

            case 0b110111: { // bun
               // Armazenando o PC antigo
                pc = R[29];
                // Execução do comportamento
                R[29] = R[29] + ((R[30] & 0x3FFFFFF) << 2);
                // Atualizando SR
                
                // Formatação da instrução
                sprintf(instrucao, "bun %i", R[30] & 0x3FFFFFF);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tPC=0x%08X\n", pc, instrucao, R[29] + 4);
                break;

            }

            case 0b111111: { // int
              // Parar a execução
                executa = 0;
                // Atualizando SR
                
                
                // Formatação da instrução
                sprintf(instrucao, "int 0");
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tCR=0x00000000,PC=0x00000000\n", R[29], instrucao);
                break;

            }

            default:
                
                atualizar_SR(R, ZN,ZD, SN,OV, IV, CY);
                sprintf(instrucao, "instrução inválida");
                printf("0x%08X:\t%-25s\n", R[29], instrucao);
                break;
        }

        R[29] += 4; // Incrementa o PC para a próxima instrução
        
    }

    return 0;
}
