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
        R[28] = ((proj.MEM8[R[29] + 0] << 24) | (proj.MEM8[R[29] + 1] << 16) |
            (proj.MEM8[R[29] + 2] << 8) | (proj.MEM8[R[29] + 3] << 0)) |
            proj.MEM32[R[29] >> 2];

        uint8_t opcode = (R[28] >> 26) & 0x3F;

        switch (opcode) {
            case 0b000000: // mov
                z = (R[28] >> 21) & 0x1F;
                l = R[28] & 0x1FFFFF;
                R[z] = l;
                sprintf(instrucao, "mov R%u,%u", z, l);
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;

            case 0b000001: // movs
                z = (R[28] >> 21) & 0x1F;
                l = R[28] & 0x1FFFFF;
                if (l & (1 << 20)) {
                    l |= 0xFFE00000;
                }
                R[z] = l;
                sprintf(instrucao, "movs R%u,%d", z, l);
                printf("0x%08X:\t%-25s\tR%u=0x%08X\n", R[29], instrucao, z, R[z]);
                break;

            case 0b000010: { // add
                z = (R[28] >> 21) & 0x1F;
                x = (R[28] >> 16) & 0x1F;
                y = (R[28] >> 11) & 0x1F;
                
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
           z = (R[28] >> 21) & 0x1F;
                x = (R[28] >> 16) & 0x1F;
                y = (R[28] >> 11) & 0x1F;
                

    R[z] = R[x] - R[y]; // Resultado da subtração

    // Atualização dos flags
    ZN = (R[z] == 0) ? 1 : 0; // Zero Flag (ZN)
    SN = (R[z] & (1 << 31)) ? 1 : 0; // Sign Flag (SN)

    OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31))) && ((R[z] & (1 << 31)) == (R[y] & (1 << 31)))) ? 1 : 0;

    CY = (R[x] < R[y]) ? 1 : 0; // Carry Flag (CY)

    // Atualiza o registrador de status
    atualizar_SR(R, ZN, ZD, SN, OV, IV, CY);

    // Impressão do resultado
    sprintf(instrucao, "sub R%u,R%u,R%u", z, x, y);
    printf("0x%08X:\t%-25s\tR%u=R%u-R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
    break;
}






    case 0b000100:{
        uint32_t func =(R[28]>>8)&0x7;
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



case 0b000101: { // cmp
    // Extração dos registradores envolvidos
    z = (R[28] >> 21) & 0x1F;  // Para cmp, o registrador z não será utilizado
    x = (R[28] >> 16) & 0x1F;
    y = (R[28] >> 11) & 0x1F;

    // Realiza a subtração para comparação
    int32_t resultado_cmp = R[x] - R[y];

    // Atualização dos flags
    ZN = (resultado_cmp == 0) ? 1 : 0; // Zero Flag (ZN)
    SN = (resultado_cmp & (1 << 31)) ? 1 : 0; // Sign Flag (SN)

    OV = (((R[x] & (1 << 31)) != (R[y] & (1 << 31))) && ((resultado_cmp & (1 << 31)) == (R[y] & (1 << 31)))) ? 1 : 0;

    CY = (R[x] < R[y]) ? 1 : 0; // Carry Flag (CY)

    // Atualiza o registrador de status
    atualizar_SR(R, ZN, ZD, SN, OV, IV, CY);

    // Impressão do resultado
    sprintf(instrucao, "cmp R%u,R%u", x, y);
    printf("0x%08X:\t%-25s\tSR=0x%08X\n", R[29], instrucao, R[31]);

    break;
}

case 0b000110: { // AND bit a bit
    // Extração dos registradores z, x e y
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador z (bits 25-21)
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador x (bits 20-16)
    y = (R[28] >> 11) & 0x1F; // Extrai o registrador y (bits 15-11)

    // Realiza a operação AND bit a bit
    R[z] = R[x] & R[y];

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;              // ZN é 1 se R[z] for 0
    SN = (R[z] & (1 << 31)) ? 1 : 0;       // SN é 1 se o bit 31 de R[z] for 1 (número negativo)

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, 0, 0, 0);   // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "and R%u,R%u,R%u", z, x, y);
    printf("0x%08X:\t%-25s\tR%u=R%u&R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
    break;
}

case 0b000111: { // OR bit a bit
    // Extração dos registradores z, x e y
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador z (bits 25-21)
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador x (bits 20-16)
    y = (R[28] >> 11) & 0x1F; // Extrai o registrador y (bits 15-11)

    // Realiza a operação OR bit a bit
    R[z] = R[x] | R[y];

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;              // ZN é 1 se R[z] for 0
    SN = (R[z] & (1 << 31)) ? 1 : 0;       // SN é 1 se o bit 31 de R[z] for 1 (número negativo)

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, 0, 0, 0);   // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "or R%u,R%u,R%u", z, x, y);
    printf("0x%08X:\t%-25s\tR%u=R%u|R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
    break;
}



case 0b001000: { // NOT bit a bit
    // Extração dos registradores z e x
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador z (bits 25-21)
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador x (bits 20-16)

    // Realiza a operação NOT bit a bit
    R[z] = ~R[x]; // Inverte todos os bits de R[x] e armazena o resultado em R[z]

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;              // ZN é 1 se R[z] for 0
    SN = (R[z] & (1 << 31)) ? 1 : 0;       // SN é 1 se o bit 31 de R[z] for 1 (número negativo)

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, 0, 0, 0);   // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "not R%u,R%u", z, x);
    printf("0x%08X:\t%-25s\tR%u=~R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, R[z], R[31]);
    break;
}


case 0b001001: { // XOR bit a bit
    // Extração dos registradores z, x e y
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador z (bits 25-21)
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador x (bits 20-16)
    y = (R[28] >> 11) & 0x1F; // Extrai o registrador y (bits 15-11)

    // Realiza a operação XOR bit a bit
    R[z] = R[x] ^ R[y]; // Realiza o XOR entre R[x] e R[y] e armazena o resultado em R[z]

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;              // ZN é 1 se R[z] for 0
    SN = (R[z] & (1 << 31)) ? 1 : 0;       // SN é 1 se o bit 31 de R[z] for 1 (número negativo)

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, 0, 0, 0);   // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "xor R%u,R%u,R%u", z, x, y);
    printf("0x%08X:\t%-25s\tR%u=R%u^R%u=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, y, R[z], R[31]);
    break;
}


case 0b010010: { // ADI (Adicionar Imediato)
    // Extração dos registradores e valor imediato
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador destino R[z]
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador fonte R[x]
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Extrai o valor imediato i, 16 bits com sinal

    // Realiza a operação de adição
    uint64_t resultado = (uint64_t)R[x] + i; // Adição com valor imediato

    R[z] = (uint32_t)resultado; // Armazena o resultado em R[z]

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;                // ZN é 1 se R[z] for 0
    SN = (R[z] & (1 << 31)) ? 1 : 0;         // SN é 1 se o bit 31 de R[z] for 1 (número negativo)
    OV = (((R[x] & (1 << 31)) != (i & (1 << 15))) && ((R[z] & (1 << 31)) != (R[x] & (1 << 31)))) ? 1 : 0; // Verificação de overflow
    CY = (resultado > 0xFFFFFFFF) ? 1 : 0;  // CY é 1 se houver carry-out

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, OV, CY, 0); // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "addi R%u,R%u,%d", z, x, i);
    printf("0x%08X:\t%-25s\tR%u=R%u+%d=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, i, R[z], R[31]);
    break;
}




case 0b010011: { // SUBI (Subtração Imediata)
    // Extração dos registradores e valor imediato
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador destino R[z]
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador fonte R[x]
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Extrai o valor imediato i, 16 bits com sinal

    // Realiza a operação de subtração
    uint64_t resultado = (uint64_t)R[x] - i; // Subtração com valor imediato

    R[z] = (uint32_t)resultado; // Armazena o resultado em R[z]

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;                // ZN é 1 se R[z] for 0
    SN = (R[z] & (1 << 31)) ? 1 : 0;         // SN é 1 se o bit 31 de R[z] for 1 (número negativo)
    OV = (((R[x] & (1 << 31)) != (i & (1 << 15))) && ((R[z] & (1 << 31)) != (R[x] & (1 << 31)))) ? 1 : 0; // Verificação de overflow
    CY = (resultado > 0xFFFFFFFF) ? 1 : 0;  // CY é 1 se houver carry-out

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, OV, CY, 0); // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "subi R%u,R%u,%d", z, x, i);
    printf("0x%08X:\t%-25s\tR%u=R%u-%d=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, i, R[z], R[31]);
    break;
}

case 0b010100: { // MULI (Multiplicação Imediata)
    // Extração dos registradores e valor imediato
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador destino R[z]
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador fonte R[x]
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Extrai o valor imediato i, 16 bits com sinal

    // Realiza a operação de multiplicação
    int64_t resultado = (int64_t)R[x] * i; // Multiplicação com valor imediato, resultado em 64 bits

    // Armazena os 32 bits menos significativos do resultado em R[z]
    R[z] = (uint32_t)resultado;

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;                  // ZN é 1 se R[z] for 0
    OV = ((resultado >> 32) != 0) ? 1 : 0;     // OV é 1 se a parte superior (bits 63-32) do resultado for diferente de 0

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, 0, 0, OV, 0); // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "muli R%u,R%u,%d", z, x, i);
    printf("0x%08X:\t%-25s\tR%u=R%u*%d=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, i, R[z], R[31]);
    break;
}

case 0b010101: { // DIVI (Divisão Imediata)
    // Extração dos registradores e valor imediato
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador destino R[z]
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador fonte R[x]
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Extrai o valor imediato i, 16 bits com sinal

    // Atualiza o flag ZD (Zero Divisor) e verifica divisão por zero
    ZD = (i == 0) ? 1 : 0;

    // Se o divisor é zero, a operação não é válida
    if (i == 0) {
        printf("Erro: Divisão por zero.\n");
        break;
    }

    // Realiza a operação de divisão
    int64_t resultado = (int64_t)R[x] / i; // Divisão com valor imediato, resultado em 64 bits

    // Armazena o resultado em R[z]
    R[z] = (uint32_t)resultado;

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;  // ZN é 1 se R[z] for 0
    OV = 0;                     // OV é sempre 0 para essa operação

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, ZD,0, 0, 0, CY); // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "divi R%u,R%u,%d", z, x, i);
    printf("0x%08X:\t%-25s\tR%u=R%u/%d=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, i, R[z], R[31]);
    break;
}

case 0b010110: { // MODI (Resto Imediato)
    // Extração dos registradores e valor imediato
    z = (R[28] >> 21) & 0x1F; // Extrai o registrador destino R[z]
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador fonte R[x]
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Extrai o valor imediato i, 16 bits com sinal

    // Atualiza o flag ZD (Zero Divisor) e verifica divisão por zero
    ZD = (i == 0) ? 1 : 0;

    // Se o divisor é zero, a operação não é válida
    if (i == 0) {
        printf("Erro: Divisão por zero.\n");
        break;
    }

    // Realiza a operação de resto
    int64_t resultado = (int64_t)R[x] % i; // Resto da divisão com valor imediato, resultado em 64 bits

    // Armazena o resultado em R[z]
    R[z] = (uint32_t)resultado;

    // Atualização dos flags de status
    ZN = (R[z] == 0) ? 1 : 0;  // ZN é 1 se R[z] for 0
    OV = 0;                     // OV é sempre 0 para essa operação

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, ZD, 0,0, 0, 0); // A função atualizar_SR manipula o registrador de status (R[31])

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "modi R%u,R%u,%d", z, x, i);
    printf("0x%08X:\t%-25s\tR%u=R%u%%%d=0x%08X, SR=0x%08X\n", R[29], instrucao, z, x, i, R[z], R[31]);
    break;
}

case 0b010111: { // CMPI (Comparação Imediata)
    // Extração dos registradores e valor imediato
    x = (R[28] >> 16) & 0x1F; // Extrai o registrador fonte R[x]
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Extrai o valor imediato i, 16 bits com sinal

    // Realiza a operação de subtração
    int64_t resultado = (int64_t)R[x] - i; // Subtração com sinal, resultado em 64 bits

    // Atualiza o registrador de status
    ZN = (resultado == 0) ? 1 : 0;  // ZN é 1 se o resultado for 0
    SN = (resultado & (1LL << 31)) ? 1 : 0; // SN é 1 se o bit mais significativo do resultado for 1
    OV = (((R[x] & (1 << 31)) != (i & (1 << 15))) && ((resultado & (1LL << 31)) != (R[x] & (1 << 31)))) ? 1 : 0; // Overflow
    CY = (resultado < 0) ? 1 : 0; // CY é 1 se o resultado for negativo

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0,0, SN, OV, CY); // ZD não é afetado por CMPI

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "cmpi R%u,%d", x, i);
    printf("0x%08X:\t%-25s\tR%u-%d=0x%08lX, SR=0x%08X\n", R[29], instrucao, x, i, resultado, R[31]);
    break;
}



case 0b011000: { // Opcode para l8
                 z = (R[28] >> 21) & 0x1F; // Registrador destino (5 bits)
                 x = (R[28] >> 16) & 0x1F; // Registrador base (5 bits)
                 i = R[28] & 0xFFFF; // Deslocamento (16 bits)

                // Calcula o endereço de memória
                uint32_t addr = R[x] + i;

                R[z] = proj.MEM8[R[x] + i] | (((uint8_t*)(&proj.MEM32[(R[x] + i) >> 2]))[3 - ((R[x] + i) % 4)]);
                    sprintf(instrucao, "l8 R%u,[R%u+0x%04X]", z, x, i);
                    printf("0x%08X:\t%-25s\tR%u=0x%02X, SR=0x%08X\n", pc, instrucao, z, R[z], R[31]);
                
                break;
            }



case 0b011001: { // Opcode para l16
    z = (R[28] >> 21) & 0x1F; // Registrador destino (5 bits)
    x = (R[28] >> 16) & 0x1F; // Registrador base (5 bits)
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Deslocamento (16 bits, com sinal)

    // Calcula o endereço de memória
    uint32_t addr = R[x] + i;

    // Verifica se o endereço está dentro dos limites da memória
    if (addr < (32 * 1024 - 1)) { // 32 KB - 1 para garantir que não acesse além do limite
        // Leitura dos 16 bits da memória
        uint16_t valor_memoria = (proj.MEM8[addr] << 8) | proj.MEM8[addr + 1];
        
        // Armazena o valor lido no registrador
        R[z] = valor_memoria;
    } else {
        // Endereço fora dos limites da memória, tratar erro
        fprintf(stderr, "Erro: Endereço de memória fora dos limites.\n");
        R[z] = 0; // Pode definir como 0 ou outro valor de erro
    }

    // Atualiza os flags de status
    ZN = (R[z] == 0) ? 1 : 0; // Zero Flag (ZN)
    SN = (R[z] & (1 << 15)) ? 1 : 0; // Sign Flag (SN)

    // Atualiza o registrador de status (SR)
    atualizar_SR(R, ZN, 0, SN, 0,0, 0); // ZD, OV e CY não são afetados por l16

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "l16 R%u,[R%u+0x%04X]", z, x, i);
    printf("0x%08X:\t%-25s\tR%u=0x%04X, SR=0x%08X\n", pc, instrucao, z, R[z], R[31]);
    break;
}






            case 0b011010: { // l32
                z = (R[28] & (0b11111 << 21)) >> 21;
				x = (R[28] & (0b11111 << 16)) >> 16;
				i = R[28] & 0xFFFF;

                uint32_t endereco = (R[x] + l) >> 2;
                R[z] = ((proj.MEM8[((R[x] + i) << 2) + 0] << 24) | (proj.MEM8[((R[x] + i) << 2) + 1] << 16) | (proj.MEM8[((R[x] + i) << 2) + 2] << 8) | (proj.MEM8[((R[x] + i) << 2) + 3] << 0)) | proj.MEM32[R[x] + i];
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
            


case 0b011011: { // Opcode para s8
    z = (R[28] >> 21) & 0x1F; // Registrador origem (5 bits)
    x = (R[28] >> 16) & 0x1F; // Registrador base (5 bits)
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Deslocamento (16 bits, com sinal)

    // Calcula o endereço de memória
    uint32_t addr = R[x] + i;

    // Verifica se o endereço está dentro dos limites da memória
     // 32 KB, sem o -1 pois não há leitura adicional
        // Armazena os 8 bits do registrador R[z] na memória
        proj.MEM8[addr] = (uint8_t)R[z];

        // Atualiza os flags de status
        ZN = (proj.MEM8[addr] == 0) ? 1 : 0; // Zero Flag (ZN)
        SN = (proj.MEM8[addr] & (1 << 7)) ? 1 : 0; // Sign Flag (SN), considerando 8 bits

        // Atualiza o registrador de status (SR)
        atualizar_SR(R, ZN, 0, SN, 0, 0,0); // ZD, OV e CY não são afetados por s8

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "s8 [R%u+0x%04X],R%u", x, i, z);
        printf("0x%08X:\t%-25s\tMEM[0x%08X]=0x%02X, SR=0x%08X\n", pc, instrucao, addr, proj.MEM8[addr], R[31]);
    
    break;
}




case 0b011100: { // Opcode para s16
    z = (R[28] >> 21) & 0x1F; // Registrador origem (5 bits)
    x = (R[28] >> 16) & 0x1F; // Registrador base (5 bits)
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Deslocamento (16 bits, com sinal)

    // Calcula o endereço de memória
    uint32_t addr = R[x] + i;

    // Verifica se o endereço está dentro dos limites da memória
    // 32 KB
        // Armazena os 16 bits do registrador R[z] na memória
        // Armazena os 8 bits menos significativos em `addr`
        proj.MEM8[addr] = (uint8_t)(R[z] & 0xFF);
        // Armazena os 8 bits mais significativos em `addr + 1`
        proj.MEM8[addr + 1] = (uint8_t)((R[z] >> 8) & 0xFF);

        // Atualiza os flags de status
        ZN = (R[z] == 0) ? 1 : 0; // Zero Flag (ZN)
        SN = (R[z] & (1 << 15)) ? 1 : 0; // Sign Flag (SN), considerando 16 bits

        // Atualiza o registrador de status (SR)
        atualizar_SR(R, ZN, 0, SN, 0,0, 0); // ZD, OV e CY não são afetados por s16

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "s16 [R%u+0x%04X],R%u", x, i, z);
        printf("0x%08X:\t%-25s\tMEM[0x%08X]=0x%04X, SR=0x%08X\n", pc, instrucao, addr, (R[z] & 0xFFFF), R[31]);
    
    break;
}


case 0b011101: { // Opcode para s32
    z = (R[28] >> 21) & 0x1F; // Registrador origem (5 bits)
    x = (R[28] >> 16) & 0x1F; // Registrador base (5 bits)
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Deslocamento (16 bits, com sinal)

    // Calcula o endereço de memória
    uint32_t addr = R[x] + (i << 2); // Deslocamento é multiplicado por 4 (shift left 2 bits)

    // Verifica se o endereço está dentro dos limites da memória

        // Armazena os 32 bits do registrador R[z] na memória
        proj.MEM32[addr >> 2] = R[z]; // Endereço em termos de palavras de 32 bits
        
        // Atualiza os flags de status
        ZN = (R[z] == 0) ? 1 : 0; // Zero Flag (ZN)
        SN = (R[z] & (1 << 31)) ? 1 : 0; // Sign Flag (SN), considerando 32 bits

        // Atualiza o registrador de status (SR)
        atualizar_SR(R, ZN, 0, SN, 0,0, 0); // ZD, OV e CY não são afetados por s32

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "s32 [R%u+0x%04X],R%u", x, (i << 2), z);
        printf("0x%08X:\t%-25s\tMEM[0x%08X]=0x%08X, SR=0x%08X\n", pc, instrucao, addr, R[z], R[31]);
    
    break;
}

case 0b101010: { // Opcode para bae
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição AE (Carry Flag CY = 0)
    if (CY == 0) {
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    }

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "bae 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b101011: { // Opcode para bat
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição AT (ZN = 0 e CY = 0)
    if (ZN == 0 && CY == 0) {
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    }

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "bat 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b101100: { // Opcode para bbe
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição BE (ZN = 1 ou CY = 1)
    if (ZN == 1 || CY == 1) {
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    }

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "bbe 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b101101: { // Opcode para bbt
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição BT (CY = 1)
    if (CY == 1) {
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    }

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "bbt 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b101110: { // Opcode para beq
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição EQ (ZN = 1)
    if (ZN == 1) {
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    }

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "beq 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b101111: { // Opcode para bge
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição GE (SN = OV)
    
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "bge 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b110000: { // Opcode para bgt
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Verifica a condição GT (ZN = 0 e SN = OV)
    
        // Atualiza o PC com o novo endereço se a condição for verdadeira
        R[29] = novo_pc;
    

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "bgt 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b110001: { // Opcode para biv
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

    // Atualiza o PC com o novo endereço
    R[29] = novo_pc;

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "biv 0x%08X", novo_pc);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    break;
}

case 0b110010: { // Opcode para ble
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Verifica a condição LE (Less or Equal)
    int condition_met = (R[31] & 0x01) || ((R[31] & 0x02) != (R[31] & 0x04)); // ZN = 1 ∨ (SN ≠ OV)

    if (condition_met) {
        // Calcula o novo endereço para o PC
        uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

        // Atualiza o PC com o novo endereço
        R[29] = novo_pc;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "ble 0x%08X", novo_pc);
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    } else {
        // Se a condição não for atendida, apenas imprime a instrução sem alterar o PC
        sprintf(instrucao, "ble (não desviado)");
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    }
    break;
}

case 0b110011: { // Opcode para blt
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Verifica a condição LT (Less Than)
    int condition_met = (R[31] & 0x02) != (R[31] & 0x04); // SN ≠ OV

    if (condition_met) {
        // Calcula o novo endereço para o PC
        uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

        // Atualiza o PC com o novo endereço
        R[29] = novo_pc;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "blt 0x%08X", novo_pc);
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    } else {
        // Se a condição não for atendida, apenas imprime a instrução sem alterar o PC
        sprintf(instrucao, "blt (não desviado)");
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    }
    break;
}

case 0b110100: { // Opcode para bne
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Verifica a condição NE (Not Equal)
    int condition_met = (R[31] & 0x01) == 0; // ZN = 0

    if (condition_met) {
        // Calcula o novo endereço para o PC
        uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

        // Atualiza o PC com o novo endereço
        R[29] = novo_pc;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "bne 0x%08X", novo_pc);
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    } else {
        // Se a condição não for atendida, apenas imprime a instrução sem alterar o PC
        sprintf(instrucao, "bne (não desviado)");
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    }
    break;
}

case 0b110101: { // Opcode para bni
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Verifica a condição NI (Not Invalid)
    int condition_met = (R[31] & 0x02) == 0; // IV = 0, assumindo que IV é o bit 1 do SR (Status Register)

    if (condition_met) {
        // Calcula o novo endereço para o PC
        uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

        // Atualiza o PC com o novo endereço
        R[29] = novo_pc;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "bni 0x%08X", novo_pc);
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    } else {
        // Se a condição não for atendida, apenas imprime a instrução sem alterar o PC
        sprintf(instrucao, "bni (não desviado)");
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    }
    break;
}

case 0b110110: { // Opcode para bnz
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Verifica a condição NZ (Not Zero)
    int condition_met = (R[31] & 0x01) == 0; // ZD = 0, assumindo que ZD é o bit 0 do SR (Status Register)

    if (condition_met) {
        // Calcula o novo endereço para o PC
        uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

        // Atualiza o PC com o novo endereço
        R[29] = novo_pc;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "bnz 0x%08X", novo_pc);
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    } else {
        // Se a condição não for atendida, apenas imprime a instrução sem alterar o PC
        sprintf(instrucao, "bnz (não desviado)");
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    }
    break;
}

case 0b111000: { // Opcode para bzd
    // Decodifica o deslocamento imediato (i) do código de operação
    int32_t i = (int32_t)(R[28] & 0x03FFFFFF); // 26 bits, com sinal

    // Ajusta o deslocamento (i) para o formato de deslocamento de 32 bits
    i = (i << 2); // Multiplica por 4 (shift left 2 bits)

    // Verifica a condição ZD (Zero Detect)
    int condition_met = (R[31] & 0x02) != 0; // ZD é o bit 1 do SR (Status Register)

    if (condition_met) {
        // Calcula o novo endereço para o PC
        uint32_t novo_pc = R[29] + 4 + i; // R[29] é o PC atual

        // Atualiza o PC com o novo endereço
        R[29] = novo_pc;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "bzd 0x%08X", novo_pc);
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    } else {
        // Se a condição não for atendida, apenas imprime a instrução sem alterar o PC
        sprintf(instrucao, "bzd (não desviado)");
        printf("0x%08X:\t%-25s\tPC=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[31]);
    }
    break;
}



            case 0b110111: { // bun
               // Armazenando o PC antigo
                pc = R[29];
                // Execução do comportamento
                R[29] = R[29] + ((R[28] & 0x3FFFFFF) << 2);
                // Atualizando SR
                
                // Formatação da instrução
                sprintf(instrucao, "bun %i", R[28] & 0x3FFFFFF);
                // Formatação de saída em tela (deve mudar para o arquivo de saída)
                printf("0x%08X:\t%-25s\tPC=0x%08X\n", pc, instrucao, R[29] + 4);
                break;
            }

case 0b011110: { // Opcode para call
    // Decodifica os campos do código de operação
    uint8_t x = (R[28] >> 16) & 0x1F; // Registrador base (5 bits)
    int16_t i = (int16_t)(R[28] & 0xFFFF); // Deslocamento imediato (16 bits)

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[x] + 16 + (i << 2); // i é deslocado 2 bits à esquerda (multiplicado por 4)

    // Armazena o endereço de retorno na pilha
    proj.MEM32[R[30] / 4] = R[29] + 4; // MEM[SP] = PC + 4, onde R[30] é o SP (Stack Pointer)
    
    // Decrementa o SP
    R[30] -= 4; // Atualiza o Stack Pointer (SP)

    // Atualiza o PC com o novo endereço
    R[29] = novo_pc; // Atualiza o PC (Program Counter)

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "call R%u+0x%04X", x, i << 2);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SP=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[30], R[31]);
    
    break;
}

case 0b111001: { // Opcode para call (Tipo S)
    // Decodifica o deslocamento imediato (25 bits)
    uint32_t i = R[28] & 0x03FFFFFF;

    // Calcula o novo endereço para o PC
    uint32_t novo_pc = R[29] + 4 + (i << 2); // i é deslocado 2 bits à esquerda

    // Armazena o endereço de retorno na pilha
    proj.MEM32[R[30] / 4] = R[29] + 4; // MEM[SP] = PC + 4, onde R[30] é o SP (Stack Pointer)
    
    // Atualiza o SP (decrementa em 4)
    R[30] -= 4;

    // Atualiza o PC com o novo endereço
    R[29] = novo_pc;

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "call 0x%08X", i << 2);
    printf("0x%08X:\t%-25s\tPC=0x%08X, SP=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[30], R[31]);
    
    break;
}

case 0b011111: { // Opcode para ret (Tipo F)
    // Atualiza o ponteiro de pilha (SP) - incrementa em 4
    R[30] += 4;

    // Atualiza o PC com o valor armazenado na memória no endereço indicado pelo SP
    R[29] = proj.MEM32[R[30] / 4]; // MEM[SP] é acessado com o offset ajustado para 32 bits

    // Impressão da operação realizada e do estado dos registradores
    sprintf(instrucao, "ret");
    printf("0x%08X:\t%-25s\tPC=0x%08X, SP=0x%08X, SR=0x%08X\n", R[29], instrucao, R[29], R[30], R[31]);
    
    break;
}

case 0b001010: { // Opcode para push (Tipo U)
    // Decodifica os campos do opcode
    uint8_t i = (R[28] >> 21) & 0x1F; // Registrador a ser empilhado (5 bits)
    
    // Verifica se o registrador a ser empilhado é válido (i != 0)
    if (i != 0) {
        // Atualiza o ponteiro de pilha (SP) - decrementa em 4
        R[30] -= 4;

        // Armazena o valor do registrador especificado na memória no endereço indicado por SP
        proj.MEM32[R[30] / 4] = R[i]; // Acesso à memória com o offset ajustado para 32 bits

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "push R%u", i);
        printf("0x%08X:\t%-25s\tSP=0x%08X, MEM[SP]=0x%08X, SR=0x%08X\n", pc, instrucao, R[30], proj.MEM32[R[30] / 4], R[31]);
    }
    
    break;
}

case 0b001011: { // Opcode para pop (Tipo U)
    // Decodifica os campos do opcode
    uint8_t i = (R[28] >> 21) & 0x1F; // Registrador a ser desempilhado (5 bits)

    // Verifica se o registrador a ser desempilhado é válido (i != 0)
    if (i != 0) {
        // Lê o valor da memória no endereço indicado por SP
        R[i] = proj.MEM32[R[30] / 4]; // Acesso à memória com o offset ajustado para 32 bits

        // Atualiza o ponteiro de pilha (SP) - incrementa em 4
        R[30] += 4;

        // Impressão da operação realizada e do estado dos registradores
        sprintf(instrucao, "pop R%u", i);
        printf("0x%08X:\t%-25s\tSP=0x%08X, R%u=0x%08X, SR=0x%08X\n", pc, instrucao, R[30], i, R[i], R[31]);
    }
    
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
