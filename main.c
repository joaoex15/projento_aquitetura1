#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint32_t MEM32[32 * 1024]; // 32 KiB de memória em 32 bits
};
void atualizar_SR(uint32_t *R, uint32_t ZN, uint32_t ZD, uint32_t SN, uint32_t OV, uint32_t IV, uint32_t CY) {
    const uint32_t ZN_MASK = 0b00000000000000000000000001000000; // Zero flag
    const uint32_t ZD_MASK = 0b00000000000000000000000000100000;
    const uint32_t SN_MASK = 0b00000000000000000000000000010000; // Sign flag
    const uint32_t OV_MASK = 0b00000000000000000000000000001000; // Overflow flag
    const uint32_t IV_MASK = 0b00000000000000000000000000000100; // Instruction Valid flag
    const uint32_t CY_MASK = 0b00000000000000000000000000000001; // Carry flag

    uint32_t sr = R[31];
    sr &= ~(ZN_MASK | ZD_MASK | SN_MASK | OV_MASK | IV_MASK | CY_MASK);

    if (ZN) sr |= ZN_MASK;
    if (ZD) sr |= ZD_MASK;
    if (SN) sr |= SN_MASK;
    if (OV) sr |= OV_MASK;
    if (IV) sr |= IV_MASK;
    if (CY) sr |= CY_MASK;

    R[31] = sr;
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
                atualizar_SR(R, ZN,ZD, SN, OV, IV,CY);

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
                atualizar_SR(R, ZN,ZD, SN, OV, IV,CY);

                // Formatação da instrução
                sprintf(instrucao, "sub r%u,r%u,r%u", z, x, y);
                fprintf(saida, "0x%08X:\t%-25s\tR%u=R%u-R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
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
    break;}











            // l8
            case 0b011000:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                x = (R[30] & (0b11111 << 16)) >> 16;
                l = R[30] & 0xFFFF;
                // Execução do comportamento com MEM32 (cálculo do índice da palavra e seleção do byte big-endian)
                R[z] = ((uint8_t*)(&proj.MEM32[(R[x] + l) >> 2]))[3 - ((R[x] + l) % 4)];
                // Atualizando SR
              
                atualizar_SR(R, ZN,ZD, SN, OV, IV,CY);
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
                atualizar_SR(R, ZN,ZD, SN, OV, IV,CY);
                // Formatação da instrução
                sprintf(instrucao, "s8 R%u,R%u,%d", z, x, l);
                // Formatação de saída em arquivo
                fprintf(saida, "0x%08X:\t%-25s\tMEM32[0x%08X] = 0x%02X,SR=0x%08X\n", R[29], instrucao, R[x] + l, R[z] & 0xFF, R[31]);
                break;
            
             case 0b110111: { // bun
               // Armazenando o PC antigo
                pc = R[29];
                // Execução do comportamento
                R[29] = R[29] + ((R[30] & 0x3FFFFFF) << 2);
                // Atualizando SR
                
                // Formatação da instrução
                sprintf(instrucao, "bun %i", R[30] & 0x3FFFFFF);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                fprintf(saida,"0x%08X:\t%-25s\tPC=0x%08X\n", pc, instrucao, R[29] + 4);
                break;

            }

            case 0b111111: { // int
              // Parar a execução
                executa = 0;
                // Atualizando SR
                
                
                // Formatação da instrução
                sprintf(instrucao, "int 0");
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                fprintf(saida,"0x%08X:\t%-25s\tCR=0x00000000,PC=0x00000000\n", R[29], instrucao);
                executa = 0; // Termina a execução

                break;

            }

            // Instrução não reconhecida
            default:
                fprintf(saida, "0x%08X:\tInstrução desconhecida\n", R[29]);
                break;
        }

        // Atualiza o PC (R29) para a próxima instrução
        R[29] += 4;
    }

    // Fecha o arquivo de saída
    fclose(saida);

    return 0;
}