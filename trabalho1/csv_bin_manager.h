#ifndef CSV_BIN_MANAGER_H
    #define CSV_BIN_MANAGER_H
    #include <stdbool.h>
    #include <stdio.h>

    bool bin2txt(FILE *bin);
    bool csv_para_binario(char *csv_pathname, char *bin_name);
    void trim(char *str);
    void binarioNaTela(char *nomeArquivoBinario);
#endif