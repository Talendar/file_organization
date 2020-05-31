/*
 * ICMC - Universidade de São Paulo (USP)
 * Trabalho 2 de Organização de Arquivos
 * 
 * Gabriel Nogueira
 * Lucas Yamamoto
 */


#include <stdio.h>
#include "funcionalidades.h"


/**
 * Função principal. Chamda automaticamente quando o programa é iniciado.
 */
int main(void) 
{   
    int opt = -1; scanf(" %d", &opt);   // Buffer de entrada do menu
    char bin_pathname[64];              // Nome do arquivo binário

    //funcionalidade 1
    if(opt == 1) {
        char csv_pathname[64];                              
        scanf(" %s %s", csv_pathname, bin_pathname);        // Lê o nome dos arquivos binário e csv
        func1(csv_pathname, bin_pathname);                  // Executa a funcionalidade 1
    }
    //outras funcionalidades
    else {
        scanf(" %s", bin_pathname);                         // Lê o nome do arquivo binário
        FILE *bin = fopen(bin_pathname, "rb+");             // Abre o arquivo binário
        
        if(bin != NULL) {
            if(opt == 2) 
                func2(bin);                                 // Executa a funcionalidade 2
            else if(opt == 3)
                func3(bin);                                 // Executa a funcionalidade 3
            else if(opt == 4)
                func4(bin);                                 // Executa a funcionalidade 4
            else if(opt == 5) {
                func5(bin_pathname, bin);                   // Executa a funcionalidade 5
                bin = NULL;                                 // Necessário para que não se tente fechar o arquivo novamente (ele já foi fechado na func5)
            }
            else if(opt == 6) {
                func6(bin_pathname, bin);                                 // Executa a funcionalidade 6
                bin = NULL;
            }
            else if(opt == 7) {
                func7(bin_pathname, bin);                                 // Executa a funcionalidade 7
                bin = NULL;
            }
            if(bin != NULL)
                fclose(bin);
        }
        else 
            printf("Falha no processamento do arquivo.");   // Erro: arquivo não encontrado
    }

    return 0;
}
