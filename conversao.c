void conversao(struct projeto *proj, int index) {
    int ind_h = 2;  // Começa após o prefixo '0x'
    int ind_b = index * 32;  // Índice para o binário

    // Inicializa o valor do MEM32 para 0
    uint32_t valor_binario = 0;
    int bit_count = 0;

    while (proj->hexcs[index][ind_h] != '\0') {
        char hex = proj->hexcs[index][ind_h];
        int bin[4] = {0, 0, 0, 0}; // 4 bits

        switch (hex) {
            case '0': memcpy(bin, (int[]){0, 0, 0, 0}, 4 * sizeof(int)); break;
            case '1': memcpy(bin, (int[]){0, 0, 0, 1}, 4 * sizeof(int)); break;
            case '2': memcpy(bin, (int[]){0, 0, 1, 0}, 4 * sizeof(int)); break;
            case '3': memcpy(bin, (int[]){0, 0, 1, 1}, 4 * sizeof(int)); break;
            case '4': memcpy(bin, (int[]){0, 1, 0, 0}, 4 * sizeof(int)); break;
            case '5': memcpy(bin, (int[]){0, 1, 0, 1}, 4 * sizeof(int)); break;
            case '6': memcpy(bin, (int[]){0, 1, 1, 0}, 4 * sizeof(int)); break;
            case '7': memcpy(bin, (int[]){0, 1, 1, 1}, 4 * sizeof(int)); break;
            case '8': memcpy(bin, (int[]){1, 0, 0, 0}, 4 * sizeof(int)); break;
            case '9': memcpy(bin, (int[]){1, 0, 0, 1}, 4 * sizeof(int)); break;
            case 'A': case 'a': memcpy(bin, (int[]){1, 0, 1, 0}, 4 * sizeof(int)); break;
            case 'B': case 'b': memcpy(bin, (int[]){1, 0, 1, 1}, 4 * sizeof(int)); break;
            case 'C': case 'c': memcpy(bin, (int[]){1, 1, 0, 0}, 4 * sizeof(int)); break;
            case 'D': case 'd': memcpy(bin, (int[]){1, 1, 0, 1}, 4 * sizeof(int)); break;
            case 'E': case 'e': memcpy(bin, (int[]){1, 1, 1, 0}, 4 * sizeof(int)); break;
            case 'F': case 'f': memcpy(bin, (int[]){1, 1, 1, 1}, 4 * sizeof(int)); break;
            default:
                printf("Caractere hexadecimal inválido: %c\n", hex);
                return;
        }

        // Atualiza o valor do MEM32
        valor_binario = (valor_binario << 4) | (bin[0] << 3) | (bin[1] << 2) | (bin[2] << 1) | bin[3];
        bit_count += 4;

        // Quando 32 bits foram preenchidos
        if (bit_count == 32) {
            proj->MEM32[index] = valor_binario;
            valor_binario = 0; // Reseta o valor para o próximo bloco de 32 bits
            bit_count = 0;
        }

        ind_h++; // Avança o índice hexadecimal
    }

    // Armazena o último valor se não for múltiplo de 32 bits
    if (bit_count > 0) {
        proj->MEM32[index] = valor_binario;
    }

    // Preenche MEM8 com 8 bits a partir de MEM32
    for (int i = 0; i < 4; i++) {
        proj->MEM8[index * 4 + i] = (uint8_t)(proj->MEM32[index] >> (24 - i * 8));
    }
}

