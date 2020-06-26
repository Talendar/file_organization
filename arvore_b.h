/**
 * 
 */

#ifndef ARVORE_B_H
    #define ARVORE_B_H
    #include <stdbool.h>
    #include <stdio.h>

    /* Constantes */
    #define MAX_CHAVES 5                // número máximo de chaves por página da árvore (ordem da árvore menos 1)
    #define MIN_CHAVES 3                // número mínimo de chaves por página da árvore (teto de (MAX_CHAVES + 1)/2)
    #define NIL -1                      // constante para indicar um valor nulo ou inválido
    #define LIXO_CHAR '$'               // caractere que indica lixo de memória
    #define PAGESIZE 72                 // tamanho, em bytes, de um registro da árvore-B
    #define HEADERSIZE 72               // tamanho, em bytes, de um cabeçalho da árvore-B

    /* Structs */
    typedef struct BTCabecalho BTCabecalho;
    typedef struct BTItem BTItem;
    typedef struct BTPagina BTPagina;

    /* Criação do índice */
    bool bt_criar(FILE *dados, char *bt_pathname);

    /* Leitura */
    BTCabecalho *bt_ler_cabecalho(FILE *bt);
    BTPagina* bt_ler_pagina(int rrn, FILE *bt);
    
    /* Escrita */
    void bt_escrever_cabecalho(BTCabecalho *cab, FILE *bt);
    void bt_escrever_pagina(BTPagina *p, int rrn, FILE *bt);
    void bt_nova_raiz(BTItem *item, int filho_esq, int filho_dir, FILE *bt, BTCabecalho *cab);

    /* Busca */
    int bt_busca(int *resultado, int chave, FILE *bt);

    /* Outros */
    BTPagina* bt_nova_pagina();
    int bt_procurar_na_pagina(BTItem *item, BTPagina *p);
#endif