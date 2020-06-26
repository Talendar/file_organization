/**
 *
 */


#include "arvore_b.h"
#include "registro_pessoa.h"
#include <stdlib.h>


/* Declaração de funções estáticas (locais) */
static bool bt_inserir_aux(int rrn, BTItem *item, int *promo_filho_dir, BTItem *promo_item, FILE *bt, BTCabecalho *cab);
static void bt_inserir_na_pagina(BTItem *item, int filho_dir, BTPagina *p);
static BTPagina* bt_split(BTItem *item, int filho_dir, BTPagina *p_antiga, BTItem *promo_item, int *promo_filho_dir, FILE *bt, BTCabecalho *cab);
static int bt_busca_aux(int *resultado, int rrn, int chave, FILE *bt);

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

    BTItem *itens[MAX_CHAVES];      // itens armazenados pelo nó/página
    int filhos[MAX_CHAVES + 1];     // RRNs dos filhos do nó/página
};


/**
 * @brief 
 * 
 * @param dados 
 * @param bt 
 */
bool bt_criar(FILE *dados, char *bt_pathname) 
{
    /* Criando arquivo de índice */
    FILE *bt = fopen(bt_pathname, "wb+");
    if(bt == NULL)
        return false;

    BTCabecalho cab = { .status = '0', .noRaiz = NIL, .nroNiveis = 0, .proxRRN = 0, .nroChaves = 0 };
    bt_escrever_cabecalho(&cab, bt);

    /* Quantidade de registros a serem lidos do arquivo de dados */
    RegistroCabecalho *dados_cab = ler_cabecalho_bin(dados);
    int regs_inseridos = qnt_registros_inseridos(dados_cab), regs_removidos = qnt_registros_removidos(dados_cab); 
    free(dados_cab); dados_cab = NULL;

    /* Inserções na árvore-B */
    if((regs_inseridos - regs_removidos) > 0) 
    {
        for(int i = 0; i < regs_inseridos; i++) {
            RegistroPessoa *rp = ler_registro_bin(dados);
            
            if(rp != NULL) {
                BTItem item = { .chave = registro_idNascimento(rp), .ponteiro = i};
                
                /* Inserção em árvore-B vazia */
                if(cab.nroChaves == 0)
                    bt_nova_raiz(&item, NIL, NIL, bt, &cab);    // cria a 1a página raiz do índice
                /* Inserção em árvore-B não-vazia */
                else {
                    int promo_rrn;                                  // RRN promovido de baixo
                    BTItem promo_item;                              // item promovido de baixo 

                    bool promocao = bt_inserir_aux(cab.noRaiz, &item, &promo_rrn, &promo_item, bt, &cab);                        
                    if(promocao)
                        bt_nova_raiz(&promo_item, cab.noRaiz, promo_rrn, bt, &cab);      // criando nova raiz (árvore aumentando de nível)
                }
            }
        }
    }

    /* Escrevendo cabeçalho atualizado */
    cab.status = '1';
    bt_escrever_cabecalho(&cab, bt);
    return true;
}


/**
 * Função principal de inserção de um novo item no índice árvore-B.
 * 
 * @param rrn RRN da página na qual se tentará fazer a inserção.
 * @param item item a ser inserido.
 * @param promo_filho_dir RRN da página filha promovida deste para o próximo nível.
 * @param promo_item item promovido deste deste para o próximo nível.
 * @param bt arquivo binário que armazena o índice árvore-B.
 * @param cab registro de cabeçalho do índice árvore-B.
 * @return true caso haja promoção vinda do nível anterior; false caso não haja promoção.
 */
static bool bt_inserir_aux(int rrn, BTItem *item, int *promo_filho_dir, BTItem *promo_item, FILE *bt, BTCabecalho *cab)
{
    /* Variáveis locais importantes */
    BTPagina *p = NULL,                 // página atual
             *nova_p = NULL;            // nova página, criada caso haja split
    bool promocao = false;              // indica se houve ou não promoção de um item do nível anterior
    int pos = NIL,                      // posição em que o item deveria estar na página
        p_b_rrn = NIL;                  // RRN promovido do nível anterior
    BTItem p_b_item;                    // item promovido do nível anterior

    /* Verificando se o nó pai é um nó folha */
    if(rrn == NIL) {
        *promo_item = *item;
        *promo_filho_dir = NIL;
        return true;                    // promove-se o item atual para que ele seja inserido em nível de folha
    }

    /* Procurando a posição do item na página */
    p = bt_ler_pagina(rrn, bt);
    pos = bt_procurar_na_pagina(item, p);

    if(pos == NIL) {
        printf("[ERRO] Tentativa de insercao de uma chave duplicada: %d\n.", item->chave);
        return false;
    }

    /* Buscando nó folha */
    promocao = bt_inserir_aux(p->filhos[pos], item, &p_b_rrn, &p_b_item, bt, cab);
    
    /* Não houve promoção */
    if(!promocao) {
        free(p);
        return false;
    }

    /* Houve promoção e há espaço para inserção nesta página */
    if(p->n < MAX_CHAVES) {
        bt_inserir_na_pagina(&p_b_item, p_b_rrn, p);
        bt_escrever_pagina(p, rrn, bt);

        cab->nroChaves++;
        free(p);
        return false;
    }

    /* Houve promoção e não há espaço para inserção nesta página: SPLIT */
    else {
        nova_p = bt_split(&p_b_item, p_b_rrn, p, promo_item, promo_filho_dir, bt, cab);
        bt_escrever_pagina(p, rrn, bt);
        bt_escrever_pagina(nova_p, *promo_filho_dir, bt);

        free(nova_p);
        free(p);

        cab->proxRRN++;
        return true;
    }
}


/**
 * Insere, mantendo a ordenação, o item e seu filho direito na página.
 * 
 * @param item 
 * @param filho_dir 
 * @param p 
 */
static void bt_inserir_na_pagina(BTItem *item, int filho_dir, BTPagina *p)
{
    /* Buscando posição de inserção e movendo, caso necessário, outros itens e filhos */
    int i = p->n;
    for(; item->chave < p->itens[i-1]->chave && i > 0; i--) {
        p->itens[i] = p->itens[i-1];
        p->filhos[i+1] = p->filhos[i];
    }

    /* Inserindo o novo item e o seu filho direito*/
    p->n++;
    p->itens[i] = item;
    p->filhos[i+1] = filho_dir;
}


/**
 * Procura em uma página a posição em que um dado item deveria estar.
 * 
 * @param item item a ser procurado.
 * @param p página que será vasculhada.
 * @return NIL, caso o item já esteja presente na página; a posição em que o item deveria estar, no caso contrário.
 */
int bt_procurar_na_pagina(BTItem *item, BTPagina *p)
{
    int pos = 0;  for(; pos < p->n && item->chave > p->itens[pos]->chave; pos++);    // procurando a posição onde o item está ou deveria estar
    if(pos < p->n && item->chave == p->itens[pos]->chave)
        return NIL;                                                                  // erro: a chave já se encontra indexada na página

    return pos;
}


/**
 * @brief 
 * 
 * @param item item a ser inserido.
 * @param filho_dir filho direito a ser inserido.
 * @param p_antiga página antiga.
 * @param promo_item item a ser promovido.
 * @param promo_filho_dir RRN a ser promovido.
 * @param bt arquivo binário que armazena o índice árvore-B.
 * @return nova página criada pelo split.
 */
static BTPagina* bt_split(BTItem *item, int filho_dir, BTPagina *p_antiga, BTItem *promo_item, int *promo_filho_dir, FILE *bt, BTCabecalho *cab)
{
    BTPagina *p_nova = bt_nova_pagina();        // nova página criada pelo split
    int meio;                                   // posição em que o split ocorrerá
    BTItem *temp_itens[MAX_CHAVES + 1];         // guarda, temporariamente, todos os itens, antes do split
    int temp_filhos[MAX_CHAVES + 2];            // guarda, temporariamente, todos os filhos, antes do split

    /* Movendo os itens e filhos para os armazenadores temporários */
    int i = 0;
    for(; i < MAX_CHAVES; i++) {
        temp_itens[i] = p_antiga->itens[i];
        temp_filhos[i] = p_antiga->filhos[i];
    }
    temp_filhos[i] = p_antiga->filhos[i];

    /* Adicionando o novo item e o seu filho direito */
    for(i = MAX_CHAVES; item->chave < temp_itens[i-1]->chave && i > 0; i--) {
        temp_itens[i] = temp_itens[i-1];
        temp_filhos[i+1] = temp_filhos[i];
    }
    temp_itens[i] = item;
    temp_filhos[i+1] = filho_dir;

    /* Promovendo o RRN da nova página */
    *promo_filho_dir = cab->proxRRN;

    /* Movendo itens e filhos dos armazenadores temporários para as páginas */
    for(i = 0; i < MIN_CHAVES; i++) {
        // 1a metade vai para a página antiga
        p_antiga->itens[i] = temp_itens[i];
        p_antiga->filhos[i] = temp_filhos[i];

        // 2a metade vai para a página nova
        p_nova->itens[i] = temp_itens[i + 1 + MIN_CHAVES];
        p_nova->filhos[i] = temp_filhos[i + 1 + MIN_CHAVES];

        // marcando a segunda metade da página antiga como vazia
        p_antiga->itens[i + MIN_CHAVES] = NULL;
        p_antiga->filhos[i + 1 + MIN_CHAVES] = NIL;
    }
    p_antiga->filhos[MIN_CHAVES] = temp_filhos[MIN_CHAVES];
    p_nova->filhos[MIN_CHAVES] = temp_filhos[i + 1 + MIN_CHAVES];

    /* Atualizando contagem de itens nas páginas */
    p_nova->n = MAX_CHAVES - MIN_CHAVES;
    p_antiga->n = MIN_CHAVES;

    /* Nível da nova página */
    p_nova->nivel = p_antiga->nivel;

    /* Promovendo item do meio e retornando a nova página */
    *promo_item = *temp_itens[MIN_CHAVES];
    return p_nova;
}


/**
 * @brief 
 * 
 * @param cab 
 * @param bt 
 */
void bt_escrever_cabecalho(BTCabecalho *cab, FILE *bt)
{
    fseek(bt, 0, SEEK_SET);                // move o ponteiro de escrita para o início do arquivo
    fwrite(&cab->status, 1, 1, bt);        // status
    fwrite(&cab->noRaiz, 4, 1, bt);        // nó raiz
    fwrite(&cab->nroNiveis, 4, 1, bt);     // número de níveis
    fwrite(&cab->proxRRN, 4, 1, bt);       // próximo RRN
    fwrite(&cab->nroChaves, 4, 1, bt);     // número de chaves

    const char lixo = LIXO_CHAR;
    for(int i = 0; i < 55; i++) {
        fwrite(&lixo, 1, 1, bt);           // lixo de memória
    }
}


/**
 * Dado um RRN, escreve uma página/nó no arquivo binário (i.e. escrita em disco).
 */
void bt_escrever_pagina(BTPagina *p, int rrn, FILE *bt) {
    fseek(bt, (rrn + 1)*72, SEEK_SET);  // aponta o ponteiro de escrita do arquivo para o offset em que se deseja escrever
    fwrite(&p->nivel, 4, 1, bt);        // escrevendo nível
    fwrite(&p->n, 4, 1, bt);            // escrevendo número de chaves

    /* Escrevendo as chaves e seus ponteiros */
    for(int i = 0; i < p->n; i++) {
        BTItem *item = p->itens[i];
        fwrite(&item->chave, 4, 1, bt);
        fwrite(&item->ponteiro, 4, 1, bt);
    }

    /* Preenchendo com lixo */
    int qnt_lixo = (MAX_CHAVES - p->n)*8;   // calculando a quantidade de bytes de lixo
    const char lixo = LIXO_CHAR;
    for(int i = 0; i < qnt_lixo; i++) 
        fwrite(&lixo, 1, 1, bt);

    /* Escrevendo ponteiros para filhos */
    for(int i = 0; i <= MAX_CHAVES; i++)
        fwrite(&p->filhos[i], 4, 1, bt);
}


/**
 * @brief Realiza busca na árvore-B a partir de uma chave
 * 
 * @param resultado RRN obtido pela busca
 * @param chave Valor da chave
 * @param bt Arquivo de índices com a árvore-B
 * @return Número de páginas acessadas
 */
int bt_busca(int *resultado, int chave, FILE *bt)
{
    if(resultado == NULL)
        return NIL;

    int raiz;
    *resultado = NIL;
    BTCabecalho *cabecalho = bt_ler_cabecalho(bt);
    if(cabecalho == NULL)
        return NIL;

    /* Salva o RRN do nó raiz e apaga o cabecalho lido */
    raiz = cabecalho->noRaiz;
    free(cabecalho);
    cabecalho = NULL;

    /* Realiza a busca recursiva */
    return bt_busca_aux(resultado, raiz, chave, bt);
}


/**
 * Função auxiliar de busca recursiva
 * 
 * @param resultado RRN encontrado na busca
 * @param rrn RRN da página a ser acessada
 * @param chave Valor da chave
 * @param bt Arquivo de índices com a árvore-B
 * @return Número de páginas acessadas
 */
static int bt_busca_aux(int *resultado, int rrn, int chave, FILE *bt) {
    if(rrn < 0)
        return 0;
    
    int i, numAcessos = 0;
    BTPagina *p = bt_ler_pagina(rrn, bt);

    if(p != NULL) {
        /* Busca pela chave */
        for(i = 0; i < p->n; i++) {
            if(p->itens[i]->chave == chave) {             // Se achar a chave
                *resultado = p->itens[i]->ponteiro;
                break;
            } else if(p->itens[i]->chave > chave) {       // Se a chave estiver em um descendente
                if(p->filhos[i] != NIL)                                                 // Se o descendente existe
                    numAcessos = bt_busca_aux(resultado, p->filhos[i], chave, bt);      // Busca no nó descendente
                break;
            } else if(i == p->n-1) {
                if(p->filhos[i+1] != NIL)                                               // Se o descendente existe
                    numAcessos = bt_busca_aux(resultado, p->filhos[i+1], chave, bt);    // Busca no nó descendente
                break;
            }
        }
        /* Conclui iteração da busca */
        numAcessos++;
        bt_apagar_pagina(&p);
    }

    return numAcessos;
}


/**
 * @brief 
 * 
 * @return BTPagina* 
 */
BTPagina* bt_nova_pagina() 
{
    BTPagina *pag = malloc(sizeof(BTPagina));
    if(pag != NULL) {
        pag->nivel = 1;
        pag->n = 0;

        for(int i = 0; i < MAX_CHAVES; i++)
            pag->itens[i] = NULL;  

        for(int i = 0; i <= MAX_CHAVES; i++)
            pag->filhos[i] = NIL;    // marca os filhos como inexistentes (nó folha)
    }

    return pag;
}


/**
 * @brief 
 * 
 * @param item 
 * @param filho_dir 
 * @param filho_esq 
 * @param bt 
 * @param cab 
 */
void bt_nova_raiz(BTItem *item, int filho_esq, int filho_dir, FILE *bt, BTCabecalho *cab)
{   
    /* Escrevendo página raiz */
    BTPagina *raiz = bt_nova_pagina();
    raiz->itens[0] = item;
    raiz->filhos[0] = filho_esq;
    raiz->filhos[1] = filho_dir;
    raiz->nivel = cab->nroNiveis + 1;
    raiz->n = 1;

    bt_escrever_pagina(raiz, cab->proxRRN, bt);
    free(raiz);

    /* Atualizando (na RAM) o cabeçalho */
    cab->nroChaves++;
    cab->nroNiveis++;
    cab->noRaiz = cab->proxRRN;
    cab->proxRRN++;
}


/**
 * Lê o cabeçalho do arquivo com a árvore-B
 * 
 * @param bt Arquivo com a árvore-B
 * @return Cabeçalho lido ou NULL caso ocorra algum erro
 */
BTCabecalho *bt_ler_cabecalho(FILE *bt)
{
    BTCabecalho *c = (BTCabecalho *) malloc(sizeof(BTCabecalho));
    if(c != NULL) {
        /* Checa se o ponteiro de leitura do arquivo está na posição correta */
        if(ftell(bt) != 0)
            fseek(bt, 0, SEEK_SET);     // Coloca o ponteiro na posição certa
        
        /* Lê o status e checa a consistencia do arquivo */
        fread(&c->status, 1, 1, bt);
        if(c->status != '1') {          // Caso não seja consistente
            free(c);                    // Apaga o cabeçalho criado
            c = NULL;
            return NULL;                // Retorna erro
        }
        /* Lê os outros campos do cabeçalho */
        fread(&c->noRaiz, 4, 1, bt);
        fread(&c->nroNiveis, 4, 1, bt);
        fread(&c->proxRRN, 4, 1, bt);
        fread(&c->nroChaves, 4, 1, bt);
    }

    return c;
}


/**
 * Lê a página/nó no RRN fornecido do arquivo de índice árvore-B.
 * 
 * @param bt arquivo binário que contém o índice árvore-B.
 * @return a página/nó lida ou NULL caso não tenha sido possível alocar a memória necessária.
 */
BTPagina* bt_ler_pagina(int rrn, FILE *bt) 
{
    BTPagina *p = bt_nova_pagina();
    if(p != NULL) {
        fseek(bt, (rrn * PAGESIZE) + HEADERSIZE, SEEK_SET); // aponta o ponteiro de leitura do arquivo para a página que deseja-se ler
        fread(&p->nivel, 4, 1, bt);                         // lendo o nível do nó
        fread(&p->n, 4, 1, bt);                             // lendo o número de chaves na página

        if(p->n > 0) {
            /* Lendo os itens armazenados na página */
            for(int i = 0; i < p->n; i++) {
                BTItem *item = malloc(sizeof(BTItem));
                if(item != NULL) {
                    fread(&item->chave, 4, 1, bt);
                    fread(&item->ponteiro, 4, 1, bt);
                    p->itens[i] = item;
                }
                else 
                    return NULL;
            }

            /* Lendo os RRNs dos filhos do nó */
            fseek(bt, (MAX_CHAVES - p->n)*8, SEEK_CUR);     // avançando o ponteiro de leitura (caso em que a página não está cheia)
            for(int i = 0; i < (p->n + 1); i++) {
                fread(&p->filhos[i], 4, 1, bt);
            }
        }
    }

    return p;
}

/**
 * Apaga a página apropriadamente e atribui NULL ao ponteiro
 * 
 * @param p Ponteiro para o ponteiro da página a ser apagada
 */
void bt_apagar_pagina(BTPagina **p) {
    if(p == NULL || *p == NULL)
        return;
    
    int i;

    /* Apaga os itens da página */
    for(i = 0; i < (*p)->n; i++) {
        free((*p)->itens[i]);
        (*p)->itens[i] = NULL;
    }

    /* Apaga a página */
    free(*p);
    *p = NULL;
}