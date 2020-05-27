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
    int opt = -1; scanf(" %d", &opt);   // Buffer de entrada do menu
    char bin_pathname[64];              // Nome do arquivo binário

    //funcionalidade 1
    if(opt == 1) {
        char csv_pathname[64];                              // Nome do arquivo csv
        scanf(" %s %s", csv_pathname, bin_pathname);        // Lê o nome dos arquivos binário e csv
        if(csv_para_binario(csv_pathname, bin_pathname))    // Executa a funcionalidade e checa se o ela ocorreu conforme esperado
            binarioNaTela(bin_pathname);                    // Imprime na tela
        else 
            printf("Falha no carregamento do arquivo.\n");
    }
    //funcionalidade 2
    else if(opt == 2) {
        scanf(" %s", bin_pathname);     // Lê o nome do arquivo binário
        bin2txt(bin_pathname);          // Executa a funcionalidade
    }
    else
        printf("Opcao invalida!\n");    // Mensagem de erro

    return 0;
}
