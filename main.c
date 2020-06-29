/*
 * ICMC - Universidade de São Paulo (USP)
 * Trabalho 2 de Organização de Arquivos
 * 
 * Gabriel Nogueira (TO_DO: completar nome e colocar NUSP)
 * Lucas Yamamoto (TO_DO: completar nome e colocar NUSP)
 */


#include <stdio.h>
#include "funcionalidades.h"
#include "csv_bin_manager.h"


/**
 * Função principal. Chamda automaticamente quando o programa é iniciado.
 */
int main(void) 
{   
    int opt = -1; scanf(" %d", &opt);   // Buffer de entrada do menu
    char bin_pathname[64];              // Nome do arquivo binário

    // funcionalidade 1
    if(opt == 1) {
        char csv_pathname[64];                              
        scanf(" %s %s", csv_pathname, bin_pathname);        // Lê o nome dos arquivos binário e csv
        func1(csv_pathname, bin_pathname);                  // Executa a funcionalidade 1
    }
    // outras funcionalidades
    else {
        scanf(" %s", bin_pathname);                         // Lê o nome do arquivo binário
        FILE *bin = fopen(bin_pathname, "rb+");             // Abre o arquivo binário

        if(bin != NULL) {
            /* Funcionalidade 2 */
            if(opt == 2)
                func2(bin);
            /* Funcionalidade 3 */
            else if(opt == 3)
                func3(bin);
            /* Funcionalidade 4 */
            else if(opt == 4)
                func4(bin);
            /* Funcionalidades 5 e 7 */
            else if(opt == 5 || opt == 7) {
                // funcionalidade 5
                if(opt == 5)    
                    func5(bin_pathname, bin);
                // funcionalidade 7
                else 
                    func7(bin_pathname, bin);
                
                bin = NULL;     // Necessário para que não se tente fechar o arquivo novamente (ele já foi fechado em uma das func anteriores)
            }
            /* Funcionalidades 6, 8, 9 e 10*/
            else if(opt == 6 || opt == 8 || opt == 9 || opt == 10) {
                char indice_pathname[64];  scanf(" %s", indice_pathname);    // Lê o nome do arquivo de índice
                
                // funcionalidade 8
                if(opt == 8) 
                    func8(bin, indice_pathname);
                // funcionalidades 6, 9 e 10
                else {
                    FILE *indice = fopen(indice_pathname, "rb+");
                    if(indice != NULL) {
                        // funcionalidade 6 e 10
                        if(opt == 6 || opt == 10) {
                            if(func6(bin_pathname, bin, indice_pathname, indice)) {
                                // binario na tela caso não tenha havido falhas
                                if(opt == 6)
                                    binarioNaTela(bin_pathname);            // binarioNaTela para o arquivo de dados
                                else 
                                    binarioNaTela(indice_pathname);         // binarioNaTela para o arquivo de índice
                            }
                            bin = NULL;               
                        }
                        // funcionalide 9
                        else {
                            func9(bin, indice);
                            fclose(indice);
                        }
                    }
                    else 
                        printf("Falha no processamento do arquivo.");   // Erro: arquivo de índice não encontrado
                } 
            }
            if(bin != NULL)
                fclose(bin);
        }
        else 
            printf("Falha no processamento do arquivo.");   // Erro: arquivo de dados não encontrado
    }

    return 0;
}
