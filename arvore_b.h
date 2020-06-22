/**
 * 
 */

#ifndef ARVORE_B_H
    #define ARVORE_B_H
    #include <stdbool.h>

    /* Constantes */
    #define BT_ORDEM 6              // ordem da árvore-B
    const char LIXO_CHAR = '$';     // caractere que denota lixo de memória

    #define BT_ERRO -1              // valor de retorno que indica erro na execução da função
    #define BT_SEM_PROMOCAO 0       // valor de retorno que indica que não houve promoção durante a inserção
    #define BT_PROMOCAO 1           // valor de retorno que indica que houve promoção durante a inserção

    /* Structs */
    typedef struct BTCabecalho BTCabecalho;
    typedef struct BTItem BTItem;
    typedef struct BTPagina BTPagina;

    /* Criação */
    bool bt_criar(char *in_pathname, char *out_pathname);

#endif