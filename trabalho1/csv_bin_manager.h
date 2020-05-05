#ifndef CSV_BIN_MANAGER_H
    #define CSV_BIN_MANAGER_H
    #include <stdbool.h>

    bool csv_para_binario(char *csv_pathname, char *bin_name);
    bool bin2txt(char *bin_pathname);
    void binarioNaTela(char *nomeArquivoBinario);
#endif