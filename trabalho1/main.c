/*
 * ICMC - Universidade de São Paulo (USP)
 * Trabalho 1 de Organização de Arquivos
 * 
 * Gabriel Nogueira
 * Lucas Yamamoto 
 */

#include <stdio.h>
#include "csv_bin_manager.h"

int main(void) 
{   
    int opt = -1; scanf(" %d", &opt);
    char bin_pathname[64];

    //funcionalidade 1
    if(opt == 1) {
        char csv_pathname[64];
        scanf(" %s %s", csv_pathname, bin_pathname);
        csv_para_binario(csv_pathname, bin_pathname);
        binarioNaTela(bin_pathname);
    }
    //funcionalidade 2
    else if(opt == 2) {
        scanf(" %s", bin_pathname);
        bin2txt(bin_pathname);
    }
    else
        printf("Opcao invalida!\n");

    return 0;
}
