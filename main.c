#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct projeto {
    uint32_t MEM32[32 * 1024]; // 32 KiB de memória em 32 bits
};

    #include <stdint.h>

void atualizar_SR_sem_OV(uint32_t *R, uint32_t ZN, uint32_t SN, uint32_t CY, uint32_t IV) {
    // Máscaras para cada flag
    const uint32_t ZN_MASK = 0b01000000; // Zero flag
    const uint32_t SN_MASK = 0b00010000; // Sign flag
    const uint32_t CY_MASK = 0b00000001; // Carry flag
    const uint32_t IV_MASK = 0b00000010; // Instruction Valid flag

    // Obtendo o valor atual do registrador de status
    uint32_t sr = R[31];

    // Limpar as flags específicas
    sr &= ~(ZN_MASK | SN_MASK | CY_MASK | IV_MASK);

    // Configurar as flags com base nos valores fornecidos
    if (ZN) {
        sr |= ZN_MASK; // Setar ZN
    }
    if (SN) {
        sr |= SN_MASK; // Setar SN
    }
    if (CY) {
        sr |= CY_MASK; // Setar CY
    }
    if (IV) {
        sr |= IV_MASK; // Setar IV
    }
    R[31] = (R[31] & ~0x00000004) | (ZN | SN | CY | IV);
    // Atualizar o registrador de status
}

void atualizar_SR_sem_CY(uint32_t *R, uint32_t ZN, uint32_t SN, uint32_t OV, uint32_t IV) {
    // Máscaras para cada flag
    const uint32_t ZN_MASK = 0b01000000; // Zero flag
    const uint32_t SN_MASK = 0b00010000; // Sign flag
    const uint32_t OV_MASK = 0b00001000; // Overflow flag
    const uint32_t IV_MASK = 0b00000010; // Instruction Valid flag

    // Obtendo o valor atual do registrador de status
    uint32_t sr = R[31];

    // Limpar as flags específicas
    sr &= ~(ZN_MASK | SN_MASK | OV_MASK | IV_MASK);

    // Configurar as flags com base nos valores fornecidos
    if (ZN) {
        sr |= ZN_MASK; // Setar ZN
    }
    if (SN) {
        sr |= SN_MASK; // Setar SN
    }
    if (OV) {
        sr |= OV_MASK; // Setar OV
    }
    if (IV) {
        sr |= IV_MASK; // Setar IV
    }

    // Atualizar o registrador de status
    R[31] = (R[31] & ~0x00000008) | (ZN | SN | OV | IV);
}





int main() {
    struct projeto proj;
    char linha[100];
    FILE *hex;
    int index = 0;
    uint32_t R[32] = {0};

    // Inicializa o PC (R29) com o endereço inicial
    R[29] = 0;

    hex = fopen("entrada.txt", "r");
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
        index++;
    }

    fclose(hex);

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
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
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
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;
                //add
           case 0b000010: { // Opcode para add
                    // Obtendo operandos
                    z = (R[30] >> 21) & 0x1F;
                    x = (R[30] >> 16) & 0x1F;
                    y = (R[30] >> 11) & 0x1F;

                    // Execução da adição
                    uint64_t resultado = (uint64_t)R[x] + (uint64_t)R[y];
                    R[z] = (uint32_t)resultado;

                    // Atualizando flags
                    ZN = (R[z] == 0) ? 0xb00001 : 0xb00000; // Zero flag
                    SN = ((int32_t)R[z] < 0) ? 0xb00001 : 0xb00000; // Sign flag
                    OV = (((R[x] & (1 << 31)) == (R[y] & (1 << 31)) && (R[z] & (1 << 31)) != (R[x] & (1 << 31))) ? 0xb00001 : 0xb00000); // Overflow flag
                    CY = (resultado > 0xFFFFFFFF) ? 0xb00001 : 0xb00000; // Carry flag
                    IV = 0xb00000; // Presumindo que a operação é válida, ajuste conforme necessário

                    // Atualizando o registrador de status
                    atualizar_SR_sem_OV(R, ZN, SN, CY, IV);

                    // Formatação da instrução
                    sprintf(instrucao, "add r%u,r%u,r%u", z, x, y);
                    printf("0x%08X:\t%-25s\tR%u=R%u+R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
                    break;
                }
                        


// sub
// sub
case 0b000011: {
    // Obtendo operandos
    z = (R[30] >> 21) & 0x1F;
    x = (R[30] >> 16) & 0x1F;
    y = (R[30] >> 11) & 0x1F;

    // Execução da subtração
    int64_t resultado = (int64_t)R[x] - (int64_t)R[y];
    R[z] = (uint32_t)resultado;

    // Atualizando flags
    ZN = (R[z] == 0) ? 0x00000001 : 0x00000000; // Zero flag
    SN = ((int32_t)R[z] < 0) ? 0x00000002 : 0x00000000; // Sign flag
    OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31)) && ((R[z] & (1 << 31)) == (R[y] & (1 << 31)))) ? 0x00000004 : 0x00000000); // Overflow flag
    CY = ((resultado > 0xFFFFFFFF) || (resultado < 0)) ? 0x00000008 : 0x00000000; // Carry flag
    IV = 0x00000000; // Presumindo que a operação é válida

    // Condicional para atualizar o status
    if (SN) {  // Se o sinal está negativo
        atualizar_SR_sem_CY(R, ZN, SN, OV, IV); // Atualizar sem Carry
    } else {
        atualizar_SR_sem_OV(R, ZN, SN, CY, IV); // Atualizar sem Overflow
    }

    // Formatação da instrução
    sprintf(instrucao, "sub R%u,R%u,R%u", z, x, y);
    printf("0x%08X:\t%-25s\tR%u=R%u-R%u=0x%08X,SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
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
                ZN = (R[z] == 0) ? 1 : 0;
                SN = (R[z] & (1 << 7)) ? 1 : 0; // 8 bits
                
                // Formatação da instrução
                sprintf(instrucao, "l8 R%u,[R%u%s%i]", z, x, (l >= 0) ? ("+") : (""), l);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=MEM[0x%08X]=0x%02X\n", R[29], instrucao, z, R[x] + l, R[z]);
                break;

            // l32
            case 0b011010:
                // Obtendo operandos
                z = (R[30] & (0b11111 << 21)) >> 21;
                x = (R[30] & (0b11111 << 16)) >> 16;
                l = R[30] & 0xFFFF;
                // Execução do comportamento com MEM32
                R[z] = proj.MEM32[(R[x] + l) >> 2];
                // Atualizando SR
                
                // Formatação da instrução
                sprintf(instrucao, "l32 R%u,[R%u%s%i]", z, x, (l >= 0) ? ("+") : (""), l);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tR%u=MEM[0x%08X]=0x%08X\n", R[29], instrucao, z, (R[x] + l) << 2, R[z]);
                break;

            // bun
            case 0b110111:
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

            // int
            case 0b111111:
                // Parar a execução
                executa = 0;
                // Atualizando SR
                
                
                // Formatação da instrução
                sprintf(instrucao, "int 0");
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tCR=0x00000000,PC=0x00000000\n", R[29], instrucao);
                break;

            // Instrução desconhecida
            default:
                // Exibindo mensagem de erro
                sprintf(instrucao, "Comando desconhecido");
                IV = 1;
                
                printf("0x%08X:\t%-25s\n", R[29], instrucao);
                break;
        }

        // PC = PC + 4 (próxima instrução)
        R[29] = R[29] + 4;
    }

    return 0;
}
