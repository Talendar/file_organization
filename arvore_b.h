/**
 * 
 */

#ifndef ARVORE_B_H
    #define ARVORE_B_H
    #include <stdbool.h>
    #include <stdio.h>

    /* Constantes */
    #define BT_ORDEM 6              // ordem da árvore-B
    #define BT_ERRO -1              // valor de retorno que indica erro na execução da função
    #define BT_SEM_PROMOCAO 0       // valor de retorno que indica que não houve promoção durante a inserção
    #define BT_PROMOCAO 1           // valor de retorno que indica que houve promoção durante a inserção

    /* Structs */
    typedef struct BTCabecalho BTCabecalho;
    typedef struct BTItem BTItem;
    typedef struct BTPagina BTPagina;

    /* Criação */
    bool bt_criar(FILE *dados_arq, char *indice_pathname);

    /* Inserção */
    bool bt_inserir(int chave, int ponteiro, FILE *bin, BTCabecalho *cab, bool ler_cab);
#endif