/**
 * 
 */


#include "arvore_b.h"
#include "registro_pessoa.h"
#include <stdlib.h>
#include <string.h>


static const char LIXO_CHAR = '$';     // caractere que denota lixo de memória


/**
 * Estrutura de um registro de cabeçalho de uma árvore-B. 
 */
struct BTCabecalho {
    char status;        // indica a consistência do arquivo de índice; pode assumir o valor 0 (arquivo inconsistente) ou 1 (arquivo consistente)
    int noRaiz,         // RRN do nó/página raiz da árvore
        nroNiveis,      // número de níveis da árvore
        proxRRN,        // valor do próximo RRN a ser usado para conter um nó/página da árvore-B
        nroChaves;      // número de chaves de busca indexadas no índice árvore-B
};


/**
 * Estrutura de um item indexado por uma árvore-B (uma chave de busca e um campo de referência).
 */
struct BTItem {
    int chave,          // chave de busca que identifica o item (registro no arquivo de dados principal)
        ponteiro;       // ponteiro para o registro, no arquivo de dados principal, que apresenta a chave de busca em questão; neste trabalho, trata-se de um RRN
};


/**
 * Estrutura de uma página/nó de uma árvore-B.
 */
struct BTPagina {
    int nivel,                      // nível no qual o nó/página se encontra
        n;                          // número de chaves presentes no nó

    BTItem *itens[BT_ORDEM - 1];    // itens armazenados pelo nó/página
    int filhos[BT_ORDEM];           // RRNs dos filhos do nó/página
};


/**
 * Cria um novo registro de cabeçalho para o índice árvore-B. O novo cabeçalho apresenta, inicialmente, status igual à 0, isto é, indica que o arquivo está inconsistente.
 * 
 * @return ponteiro para o cabeçalho criado ou NULL caso não tenha sido possível alocar a memória necessária.
 */
static BTCabecalho* bt_criar_cabecalho() 
{
    BTCabecalho *cab = malloc(sizeof(BTCabecalho));
    if(cab != NULL) {
        cab->status = '0';
        cab->noRaiz = -1;
        cab->nroNiveis = cab->proxRRN = cab->nroChaves = 0;
    }

    return cab;
}


/**
 * @brief 
 * 
 * @return BTPagina* 
 */
static BTPagina* bt_criar_pagina() 
{
    BTPagina *pag = malloc(sizeof(BTPagina));
    if(pag != NULL) {
        pag->nivel = 1;
        pag->n = 0;

        for(int i = 0; i < (BT_ORDEM - 1); i++)
            pag->itens[i] = NULL;  

        for(int i = 0; i < BT_ORDEM; i++)
            pag->filhos[i] = -1;    // marca os filhos como inexistentes (nó folha)
    }

    return pag;
}


/**
 * @brief 
 * 
 * @param chave 
 * @param ponteiro 
 * @return BTItem* 
 */
static BTItem* bt_criar_item(int chave, int ponteiro) 
{
    BTItem *item = malloc(sizeof(BTItem));
    if(item != NULL) {
        item->chave = chave;
        item->ponteiro = ponteiro;
    }

    return item;
}


/**
 * TO_DO
 * 
 * @param cab 
 * @param bin 
 */
void bt_escrever_cabecalho(BTCabecalho *cab, FILE *bin) 
{
    fseek(bin, 0, SEEK_SET);                // move o ponteiro de escrita para o início do arquivo
    fwrite(&cab->status, 1, 1, bin);        // status
    fwrite(&cab->noRaiz, 4, 1, bin);        // nó raiz
    fwrite(&cab->nroNiveis, 4, 1, bin);     // número de níveis
    fwrite(&cab->proxRRN, 4, 1, bin);       // próximo RRN
    fwrite(&cab->nroChaves, 4, 1, bin);     // número de chaves

    for(int i = 0; i < 55; i++) {
        fwrite(&LIXO_CHAR, 1, 1, bin);      // lixo de memória
    }
}


/**
 * @brief 
 * 
 * @param bin 
 * @return BTCabecalho* 
 */
BTCabecalho* bt_ler_cabecalho(FILE *bin) 
{
    BTCabecalho *cab = bt_criar_cabecalho();
    if(cab != NULL) {
        fseek(bin, 0, SEEK_SET);                                                 // move o ponteiro de escrita para o início do arquivo
        if( (fread(&cab->status, 1, 1, bin) == 1) && (cab->status == '1') ) {    // lê o campo status e verifica-o
            fread(&cab->noRaiz, 4, 1, bin);                                      // nó raiz
            fread(&cab->nroNiveis, 4, 1, bin);                                   // número de níveis
            fread(&cab->proxRRN, 4, 1, bin);                                     // número de níveis
            fread(&cab->nroChaves, 4, 1, bin);                                   // número de chaves
        }
        else {
            free(cab);
            cab = NULL;
        }
    }

    return cab;
}


/**
 * @brief 
 * 
 * @param dados_arq 
 * @param indice_pathname 
 * @return true 
 * @return false 
 */
bool bt_criar(FILE *dados_arq, char *indice_pathname) 
{
    /* Abrindo arquivos */
    FILE *indice_arq;      // ponteiro para de saída (índice árvore-B)
    if( (indice_arq = fopen(indice_pathname, "wb")) == NULL ) 
        return false;

    /* Lendo quantidade de registros no arquivo de dados */
    RegistroCabecalho *dados_cab = ler_cabecalho_bin(dados_arq);
    if(dados_cab == NULL) {
        fclose(dados_arq);
        fclose(indice_arq);
        return false;
    }
    int qnt_registros = qnt_registros_inseridos(dados_cab);     // quantidade de registros no arquivo de dados
    free(dados_cab);  dados_cab = NULL;


    /* Criando registro de cabeçalho do índice */
    BTCabecalho *indice_cab = bt_criar_cabecalho();             // criando o registro de cabeçalho do índice árvore-B
    bt_escrever_cabecalho(indice_cab, indice_arq);              // escreve o cabeçalho com status '0'

    /* Inserindo itens no índice */
    for(int i = 0; i < qnt_registros; i++) {
        RegistroPessoa *reg = ler_registro_bin(dados_arq);
        if(reg != NULL) {   // verifica se o registro foi logicamente removido
            bt_inserir(registro_idNascimento(reg), i, indice_arq, indice_cab, false);
            liberar_registro(&reg, true);
        }
    }

    /* Escrevendo cabeçalho atualizado */
    indice_cab->status = '1';
    bt_escrever_cabecalho(indice_cab, indice_arq);
    free(indice_cab);

    /* Fechando arquivo de índice e retornando */
    fclose(indice_arq);
    return true;
}


/**
 * Lê a página/nó no RRN fornecido do arquivo de índice árvore-B.
 * 
 * @param indice_arq arquivo binário que contém o índice árvore-B.
 * @return a página/nó lida ou NULL caso não tenha sido possível alocar a memória necessária.
 */
static BTPagina* bt_ler_pagina(FILE* indice_arq, int rrn) 
{
    BTPagina *p = calloc(1, sizeof(BTPagina));
    if(p != NULL) {
        fseek(indice_arq, (rrn + 1)*72, SEEK_SET);  // aponta o ponteiro de leitura do arquivo para a página que deseja-se ler
        fread(&p->nivel, 4, 1, indice_arq);         // lendo o nível do nó
        fread(&p->n, 4, 1, indice_arq);             // lendo o número de chaves na página

        if(p->n > 0) {
            /* Lendo os itens armazenados na página */
            for(int i = 0; i < p->n; i++) {
                fread(&p->itens[i]->chave, 4, 1, indice_arq);
                fread(&p->itens[i]->ponteiro, 4, 1, indice_arq);
            }

            /* Lendo os RRNs dos filhos do nó */
            fseek(indice_arq, (BT_ORDEM - p->n - 1)*8, SEEK_CUR);    // avançando o ponteiro de leitura (caso em que a página não está cheia)
            for(int i = 0; i < (p->n + 1); i++) {
                fread(&p->filhos[i], 4, 1, indice_arq);
            }
        }
    }

    return p;
}


/**
 * Retorna true caso o nó/página seja folha e false no caso contrário.
 */
static bool bt_folha(BTPagina *p) {
    return p->filhos[0] == -1;
}


/**
 * Retorna true caso a página esteja cheia (não a espaço para a inserção de uma nova chave) e false no caso contrário.
 */
static bool bt_pagina_cheia(BTPagina *p) {
    return p->n >= (BT_ORDEM - 1);
}


/**
 * Dado um RRN, escreve uma página/nó no arquivo binário (i.e. escrita em disco).
 */
static void bt_escrever_pagina(FILE *bin, BTPagina *p, int rrn) {
    fseek(bin, (rrn + 1)*72, SEEK_SET);  // aponta o ponteiro de escrita do arquivo para o offset em que se deseja escrever
    fwrite(&p->nivel, 4, 1, bin);        // escrevendo nível
    fwrite(&p->n, 4, 1, bin);            // escrevendo número de chaves

    /* Escrevendo as chaves e seus ponteiros */
    for(int i = 0; i < p->n; i++) {
        BTItem *item = p->itens[i];
        fwrite(&item->chave, 4, 1, bin);
        fwrite(&item->ponteiro, 4, 1, bin);
    }

    /* Preenchendo com lixo */
    int qnt_lixo = ((BT_ORDEM - 1) - p->n)*8;   // calculando a quantidade de bytes de lixo
    for(int i = 0; i < qnt_lixo; i++) 
        fwrite(&LIXO_CHAR, 1, 1, bin);

    /* Escrevendo ponteiros para filhos */
    for(int i = 0; i < BT_ORDEM; i++)
        fwrite(&p->filhos[i], 4, 1, bin);
}


/**
 * Libera a RAM alocada pela página.
 */
static void bt_liberar_pagina(BTPagina **p) {
    for(int i = 0; i < (*p)->n; i++) 
        free((*p)->itens[i]);

    free(*p);
    (*p) = NULL;
}


/**
 * Insere, de forma ordenada, um novo item em uma página que tenha espaço disponível.
 * 
 * TO_DO
 */
static void bt_inserir_em_pagina_com_espaco(BTItem *itens[], int n, int filhos[], BTItem *item, int filho_dir) 
{
    itens[n] = item;
    filhos[n + 1] = filho_dir;

    if(n > 0) {
        int i = n;
        while(item->chave < itens[i-1]->chave) {
            // trocando itens
            itens[i] = itens[i - 1];
            itens[i - 1] = item;

            // trocando filhos da direita
            filhos[i + 1] = filhos[i];
            filhos[i] = filho_dir;
            i--;
        }
    }
}


/**
 * TO_DO
 * 
 * @param novo_item novo item a ser inserido.
 * @param novo_filho_dir filho a direita do novo item a ser inserido.
 * @param p página de disco atual.
 * @param promo_item item promovido.
 * @return nova página de disco.
 */
static BTPagina* bt_split(BTItem *novo_item, int novo_filho_dir, BTPagina *p, BTItem **promo_item) 
{
    /* Simulando página longa com 1 item e 1 filho a mais */
    BTItem *itens_long[BT_ORDEM];   // vetor de itens capaz de guardar 1 item a mais do que páginas normais
    int filhos_long[BT_ORDEM + 1];  // vetor de RRNs capaz de guardar 1 filho a mais do que páginas normais

    memcpy(itens_long, p->itens, BT_ORDEM * sizeof(BTItem*));
    memcpy(filhos_long, p->filhos, (BT_ORDEM + 1) * sizeof(int));

    /* Inserindo novo item na página longa */
    bt_inserir_em_pagina_com_espaco(itens_long, BT_ORDEM, filhos_long, novo_item, novo_filho_dir);

    /* Criando nova página */
    BTPagina *nova_p = bt_criar_pagina();
    nova_p->nivel = p->nivel;

    /* Selecionando item a ser promovido */
    (*promo_item) = itens_long[BT_ORDEM/2];
    
    /* Copiando itens e ponteiros para filhos que precedem "promo_item" da página longa para a página "p" */
    // itens
    p->n = 0;
    for(int i = 0; i < (BT_ORDEM - 1); i++) {
        if(i < BT_ORDEM/2) {
            p->itens[i] = itens_long[i];
            p->n++;
        }
        else 
            p->itens[i] = NULL;
    }
    // filhos
    for(int i = 0; i < BT_ORDEM; i++) {
        if(i < (BT_ORDEM/2 + 1))
            p->filhos[i] = filhos_long[i];
        else 
            p->filhos[i] = -1;
    }

    /* Copiando os itens e ponteiros para filhos que sucedem "promo_item" da página longa para a nova página */
    // itens
    nova_p->n = 0;
    for(int i = 1; i < BT_ORDEM/2; i++) {
        nova_p->itens[i-1] = itens_long[i + BT_ORDEM/2];
        nova_p->n++;
    }
    // filhos
    for(int i = 1; i < (BT_ORDEM/2 + 1); i++) {
        nova_p->filhos[i-1] = filhos_long[i + BT_ORDEM/2];
    }

    /* Retornando a página criada */
    return nova_p;
}


/**
 * TO_DO 
 * 
 * @param rrn_atual RRN da página da árvore-B que está atualmente em uso (inicialmente, a raiz).
 * @param novo_item o novo item a ser inserido na árvore.
 * @param promo_item item promovido, caso a inserção resulte no particionamento e na promoção do item. 
 * @param promo_filho_dir ponteiro para o filho direito de promo_item; quando ocorre um particionamento, não somente o item promovido deve ser inserido em um nó de nível mais alto da árvore, mas também deve ser inserido o RRN da nova página criada no particionamento.
 * @param bin arquivo binário que armazena o índica árvore-B.
 * 
 * @return BT_PROMOCAO quando uma inserção é feita e um item é promovido (nó cheio, i.e., overflow).
 * @return BT_SEM_PROMOCAO quando uma inserção é feita e nenhum item é promovido (nó com espaço livre).
 * @return BT_ERRO quando a chave de um item sendo inserido já está associada a outro item presente na árvore-B. 
 */
static int bt_inserir_aux(int rrn_atual, BTItem *novo_item, BTItem **promo_item, int *promo_filho_dir, FILE *bin, BTCabecalho *cab)
{
    /* Declaração de variáveis locais importantes (inicializadas com valores padrões inválidos) */
    BTPagina *p = NULL;                // página de disco atualmente examinada pela função
    BTPagina *nova_p = NULL;           // página de disco nova resultante do particionamento
    int pos = -1;                      // posição, na página p, no qual a chave do novo item ocorre ou deveria ocorrer
    BTItem *P_B_ITEM = NULL;           // item promovido do nível inferior para ser inserido na página p
    int P_B_RRN = -1;                  // RRN do filho à direita de P_B_ITEM, a ser inserido na página p


    /* Verificando se o nó pai é folha */
    if(rrn_atual == -1) {
        (*promo_item) = novo_item;
        (*promo_filho_dir) = -1;
        return BT_PROMOCAO;            // indica que deve-se tentar inserir o item no nó pai (folha)
    }
    /* Nó pai não é folha */
    else {
        p = bt_ler_pagina(bin, rrn_atual);

        /* Procurando pela chave do item na página */
        pos = 0;
        while(pos < p->n && novo_item->chave >= p->itens[pos]->chave) {
            if(novo_item->chave == p->itens[pos]->chave) {
                bt_liberar_pagina(&p);
                return BT_ERRO;        // erro: chave duplicada
            }
            pos++;
        }

        /* Chave de busca não encontrada; procurando-a no nó filho */
        int valor_de_retorno = bt_inserir_aux(p->filhos[pos], novo_item, &P_B_ITEM, &P_B_RRN, bin, cab);

        /* Encerrar execução */
        if(valor_de_retorno == BT_SEM_PROMOCAO || valor_de_retorno == BT_ERRO) {
            return valor_de_retorno;
        }
        /* Há espaço na página (inserir sem particionamento) */
        else if(p->n < (BT_ORDEM - 1)) {
            bt_inserir_em_pagina_com_espaco(p->itens, p->n, p->filhos, P_B_ITEM, P_B_RRN);
            p->n++;
            bt_escrever_pagina(bin, p, rrn_atual);
            return BT_SEM_PROMOCAO;
        }
        /* Inserção sem particionamento, indicando item promovido */
        else {
            BTPagina *nova_p = bt_split(P_B_ITEM, P_B_RRN, p, promo_item);

            bt_escrever_pagina(bin, p, rrn_atual);
            bt_escrever_pagina(bin, nova_p, cab->proxRRN);

            cab->proxRRN++;
            return BT_PROMOCAO;
        }
    }
}   


/**
 * @brief 
 * 
 * @param chave 
 * @param ponteiro 
 * @param bin 
 * @param cab 
 * @param ler_cab 
 * @return true 
 * @return false 
 */
bool bt_inserir(int chave, int ponteiro, FILE *bin, BTCabecalho *cab, bool ler_cab) 
{   
    /* Lendo registro de cabeçalho (caso necessário) */
    if(ler_cab) {
        cab = bt_ler_cabecalho(bin);
        if(cab == NULL)
            return false;

         /* Marcando o arquivo como inconsistente */
        cab->status = '0';
        bt_escrever_cabecalho(cab, bin);
    }

    /* Criando o item a ser adicionado */
    BTItem *novo_item = bt_criar_item(chave, ponteiro); 

    /* Criando página raiz e inserindo o item nela, caso o índice esteja vazio */
    if(cab->nroChaves == 0) {
        BTPagina *raiz = bt_criar_pagina();
        raiz->itens[0] = novo_item;
        raiz->n++;

        bt_escrever_pagina(bin, raiz, cab->proxRRN);
        bt_liberar_pagina(&raiz);
        cab->noRaiz = 0;
    }
    /* Tentando inserir o item em uma página já existente */
    else {
        int rrn_raiz = cab->noRaiz;             // RRN do nó raiz
        BTItem *promo_item = NULL;              // item promovido (caso haja)
        int promo_filho_dir = -1;               // filho à direita do item promovido (caso haja)

        if( bt_inserir_aux(rrn_raiz, novo_item, &promo_item, &promo_filho_dir, bin, cab) == BT_PROMOCAO ) {
            BTPagina *raiz = bt_criar_pagina();
            raiz->itens[0] = promo_item;
            raiz->filhos[0] = rrn_raiz;
            raiz->filhos[1] = promo_filho_dir;
            raiz->nivel = cab->nroNiveis + 1;

            bt_escrever_pagina(bin, raiz, cab->proxRRN);
            bt_liberar_pagina(&raiz);

            cab->noRaiz = cab->proxRRN;
            cab->proxRRN++;
            cab->nroNiveis++;
        }
    }

    /* Atualizando número de chaves no cabeçalho */
    cab->nroChaves++;

    /* Escrevendo o cabeçalho atualizado (caso necessário) */
    if(ler_cab) {
        cab->status = '1';
        bt_escrever_cabecalho(cab, bin);
        free(cab);
    }

    /* Finalizando */
    return true;
}

