/**
 * 
 */


#include "arvore_b.h"
#include "registro_pessoa.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
 * @param in_pathname 
 * @param out_pathname 
 * @return true 
 * @return false 
 */
bool bt_criar(char *in_pathname, char *out_pathname) 
{
    /* Abrindo arquivos */
    FILE *dados_arq, *indice_arq;      // ponteiros para os arquivos de entrada (arquivo de dados) e saída (índice árvore-B)
    if( (dados_arq = fopen(in_pathname, "r")) == NULL )
        return false;
    if( (indice_arq = fopen(out_pathname, "wb")) == NULL ) {
        fclose(dados_arq);
        return false;
    }   

    /* Lendo quantidade de registros no arquivo de dados */
    RegistroCabecalho *dados_cab = ler_cabecalho_bin(dados_arq);
    if(dados_cab == NULL)
        return false;
    int qnt_registros = qnt_registros_inseridos(dados_cab);     // quantidade de registros no arquivo de dados
    free(dados_cab);


    /* Criando registro de cabeçalho do índice */
    BTCabecalho *indice_cab = bt_criar_cabecalho();             // registro de cabeçalho do índice árvore-B
    if(indice_cab == NULL)
        return false;

    /* Inserindo itens no índice */
    for(int i = 0; i < qnt_registros; i++) {
        RegistroPessoa *reg = ler_registro_bin(dados_arq);
        if(reg != NULL) {   // verifica se o registro foi logicamente removido
            //to_do: inserir item no índice
            free(reg);
        }
    }
    
    //to_do: set status to '1'; write header
}


/**
 * Lê a página/nó no RRN fornecido do arquivo de índice árvore-B.
 * 
 * @param indice_arq arquivo binário que contém o índice árvore-B.
 * @return a página/nó lida ou NULL caso não tenha sido possível alocar a memória necessária.
 */
static BTPagina* ler_pagina(FILE* indice_arq, int rrn) 
{
    BTPagina *p = malloc(sizeof(BTPagina));
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
static bool folha(BTPagina *p) {
    return p->filhos[0] == -1;
}


/**
 * Retorna true caso a página esteja cheia (não a espaço para a inserção de uma nova chave) e false no caso contrário.
 */
static bool pagina_cheia(BTPagina *p) {
    return p->n >= (BT_ORDEM - 1);
}


/**
 * Dado um RRN, escreve uma página/nó no arquivo binário (i.e. escrita em disco).
 */
static void escrever_pagina(FILE *bin, BTPagina *p, int rrn) {
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
static void liberar_pagina(BTPagina **p) {
    //to_do
    (*p) = NULL;
}


/**
 * Insere, de forma ordenada, um novo item em uma página que tenha espaço disponível.
 * 
 * TO_DO
 */
static void inserir_em_pagina_com_espaco(BTItem *itens[], int n, int filhos[], BTItem *item, int filho_dir) 
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
static BTPagina* split(BTItem *novo_item, int novo_filho_dir, BTPagina *p, BTItem **promo_item) 
{
    /* Simulando página longa com 1 item e 1 filho a mais */
    BTItem *itens_long[BT_ORDEM];   // vetor de itens capaz de guardar 1 item a mais do que páginas normais
    int filhos_long[BT_ORDEM + 1];  // vetor de RRNs capaz de guardar 1 filho a mais do que páginas normais

    memcpy(itens_long, p->itens, BT_ORDEM * sizeof(BTItem*));
    memcpy(filhos_long, p->filhos, (BT_ORDEM + 1) * sizeof(int));

    /* Inserindo novo item na página longa */
    inserir_em_pagina_com_espaco(itens_long, BT_ORDEM, filhos_long, novo_item, novo_filho_dir);

    /* Criando nova página */
    BTPagina *nova_p = bt_criar_pagina();

    /* Selecionando item a ser promovido */
    (*promo_item) = itens_long[BT_ORDEM/2];
    
    /* Copiando itens e ponteiros para filhos que precedem "promo_item" da página longa para a página "p" */
    // itens
    for(int i = 0; i < (BT_ORDEM - 1); i++) {
        if(i < BT_ORDEM/2)
            p->itens[i] = itens_long[i];
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
    for(int i = 1; i < BT_ORDEM/2; i++) {
        nova_p->itens[i-1] = itens_long[i + BT_ORDEM/2];
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
    
    static BTItem *P_B_ITEM = NULL;    // item promovido do nível inferior para ser inserido na página p
    static int P_B_RRN = -1;           // RRN do filho à direita de P_B_ITEM, a ser inserido na página p


    /* Verificando se o nó pai é folha */
    if(rrn_atual == -1) {
        (*promo_item) = novo_item;
        (*promo_filho_dir) = -1;
        return BT_PROMOCAO;            // indica que deve-se tentar inserir o item no nó pai (folha)
    }
    /* Nó pai não é folha */
    else {
        p = ler_pagina(bin, rrn_atual);

        /* Procurando pela chave do item na página */
        pos = 0;
        while(pos < p->n && novo_item->chave >= p->itens[pos]->chave) {
            if(novo_item->chave == p->itens[pos]->chave) {
                liberar_pagina(&p);
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
            inserir_em_pagina_com_espaco(p->itens, p->n, p->filhos, P_B_ITEM, P_B_RRN);
            p->n++;
            return BT_SEM_PROMOCAO;
        }
        /* Inserção sem particionamento, indicando item promovido */
        else {
            BTPagina *nova_p = split(P_B_ITEM, P_B_RRN, p, promo_item);

            escrever_pagina(bin, p, rrn_atual);
            escrever_pagina(bin, nova_p, cab->proxRRN);

            cab->proxRRN++;
            return BT_PROMOCAO;
        }
    }
}   


