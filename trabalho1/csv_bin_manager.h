#ifndef CSV_BIN_MANAGER_H
    #define CSV_BIN_MANAGER_H
    #include <stdbool.h>

    bool bin2txt(char *bin_pathname);
    bool csv_para_binario(char *csv_pathname, char *bin_name);
    void trim(char *str);
    void binarioNaTela(char *nomeArquivoBinario);
#endif