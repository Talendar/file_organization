#include "registro_pessoa.h"
#include <stdlib.h>
#include <string.h>


static void fwrite_aux(char *campo, int n, FILE *bin);
static void imprimir_checar_vazio(char *campo);


/**
 * Definição da estrutura do registro de cabeçalho.
 */
struct Cabecalho {
    char status;                        // Indica consistência do arquivo
    int RRNproxRegistro;                // Indica RRN do próximo registro a ser inserido
    int numeroRegistrosInseridos;       // Indica o número de registros inseridos
    int numeroRegistrosRemovidos;       // Indica o número de registros removidos
    int numeroRegistrosAtualizados;     // Indica o número de registros atualizados
    char lixo[111 + 1];                 // Padding
};


/**
 * Definição da estrutura de um registro de uma pessoa, lido de um arquivo csv.
 */
struct Dados {
     //TAMANHO VARIÁVEL
    char *cidadeMae,                //nome da cidade de residência da mãe
         *cidadeBebe;               //nome da cidade na qual o bebê nasceu

    //TAMANHO FIXO
    int  idNascimento,              //código sequencial que identifica univocamente cada registro do arquivo de dados
         idadeMae;                  //idade da mãe do bebê
    char dataNascimento[10+1],      //no formato AAAA-MM-DD
         sexoBebe,                  //pode assumir os valores ‘0’ (ignorado), ‘1’ (masculino) e ‘2’ (feminino)
         estadoMae[2+1],            //sigla do estado da cidade de residência da mãe)
         estadoBebe[2+1];           //sigla do estado da cidade na qual o bebê nasceu
};


/**
 * Lê um registro do tipo pessoa contido em uma linha de um arquivo csv.
 * 
 * @param line string sem \n contendo o registro; trata-se de uma linha de um arquivo csv de header pré-definido.
 * @return um ponteiro para a struct criada; retorna NULL em casos de falhas de leitura.
 */
RegistroPessoa* ler_registro_csv(char *line) 
{
    RegistroPessoa *rp = malloc(sizeof(RegistroPessoa));
    if(rp != NULL) {
        rp->cidadeMae = strsep(&line, ",");
        rp->cidadeBebe = strsep(&line, ",");
        rp->idNascimento = atoi(strsep(&line, ","));
        rp->idadeMae = atoi(strsep(&line, ","));

        if(rp->idadeMae == 0)
            rp->idadeMae = -1;

        strcpy(rp->dataNascimento, strsep(&line, ","));
        
        char *temp_sexobb = strsep(&line, ",");
        if(strlen(temp_sexobb) == 0)
            rp->sexoBebe = '0';
        else
            rp->sexoBebe = temp_sexobb[0];

        strcpy(rp->estadoMae, strsep(&line, ","));
        strcpy(rp->estadoBebe, strsep(&line, ","));
    }

    return rp;
}


/**
 * Libera a memória alocada pelo registro.
 * 
 * @param rp ponteiro para um ponteiro para o registro a ser liberado; ao final do procedimento, o ponteiro para o registro terá valor NULL.
 * @param liberar_campos_variaveis caso o registro tenha sido lido de um arquivo csv (usando-se a função strsep), este parâmetro deve receber false. Caso contrário, true.
 * @return
 */
void liberar_registro(RegistroPessoa **rp, bool liberar_campos_variaveis) 
{
    if(liberar_campos_variaveis) {   //a memória alocada pelos campos de tamanho variável de rp pode já ter sido liberada (peculiaridade do uso da função strsep)
        free((*rp)->cidadeMae);
        free((*rp)->cidadeBebe);
    }
        
    free(*rp);
    (*rp) = NULL;
}


/**
 * Grava o registro na posição atual do ponteiro de escrita do arquivo binário fornecido.
 * 
 * @param rp ponteiro para um registro.
 * @param bin ponteiro para o arquivo binário.
 * @return
 */
void registro2bin(RegistroPessoa *rp, FILE *bin) 
{
    /* Escreve cada campo para o binário */
    int sizeCidadeMae = strlen(rp->cidadeMae), sizeCidadeBebe = strlen(rp->cidadeBebe); //tamanho da cidade da mae e da cidade do bebe
    fwrite(&sizeCidadeMae, 4, 1, bin);                                                  
    fwrite(&sizeCidadeBebe, 4, 1, bin);                                                

    fwrite(rp->cidadeMae, 1, strlen(rp->cidadeMae), bin);
    fwrite(rp->cidadeBebe, 1, strlen(rp->cidadeBebe), bin);

    /* Escreve os campos de tamanho variável com padding */
    int pad = 105 - (8 + strlen(rp->cidadeMae) + strlen(rp->cidadeBebe));               //tamanho do padding
    char pad_char = '$';                                                                //byte do padding
    for(int i = 0; i < pad; i++)
        fwrite(&pad_char, 1, 1, bin);                                                   //adicionando padding ($)

    fwrite(&rp->idNascimento, 4, 1, bin);
    fwrite(&rp->idadeMae, 4, 1, bin);

    fwrite_aux(rp->dataNascimento, 10, bin);
    fwrite(&rp->sexoBebe, 1, 1, bin);
    fwrite_aux(rp->estadoMae, 2, bin);
    fwrite_aux(rp->estadoBebe, 2, bin);
}   


/**
 * Wrapper da função fwrite. Caso o valor do campo seja nulo, um caractere \0 seguido por paddings com caracteres $ são escritos no arquivo.
 * 
 * @param campo ponteiro para o o valor do campo.
 * @param n tamanho do campo.
 * @param bin arquivo binário no qual será realizada a escrita.
 * @return
 */
static void fwrite_aux(char *campo, int n, FILE *bin) 
{
    if(strlen(campo) != 0)          
        fwrite(campo, 1, n, bin);               //escreve normalmente caso o campo não seja nulo
    else {
        char null_char = '\0', pad_char = '$';  //bytes de preenchimento
        fwrite(&null_char, 1, 1, bin);          //coloca o '\0' 
        for(int i = 1; i < n; i++) 
            fwrite(&pad_char, 1, 1, bin);       //preenche com '$' 
    }
}


/**
 * Cria um novo registro de cabeçalho para o arquivo. O cabeçalho retornado possui o campo "status" igual à 0 (indica arquivo inconsistente).
 * 
 * @return um ponteiro para o novo cabeçalho.
 */
RegistroCabecalho* criar_cabecalho(void) 
{
    /* Cria o cabeçalho inicializado para um binário novo */
    RegistroCabecalho *c = malloc(sizeof(RegistroCabecalho));   // Cria o cabeçalho
    if(c != NULL) {
        /* Inicializa os campos */
        c->status = '0';         // arquivo marcado, inicialmente, como inconsistente
        c->RRNproxRegistro = 0;
        c->numeroRegistrosInseridos = 0;
        c->numeroRegistrosAtualizados = 0;
        c->numeroRegistrosRemovidos = 0;

        /* Preenche com '$' o lixo */
        for(int i = 0; i < 111; i++)
            c->lixo[i] = '$';
        c->lixo[111] = '\0';    // Coloca o '\0' no fim para usar como string
    }

    return c;
}


/**
 * Escreve o cabeçalho no início do arquivo. Ao final do procedimento, o ponteiro de escrita do arquivo irá se encontrar na posição que se segue ao final do cabeçalho.
 * 
 * @param c ponteiro para o cabeçalho.
 * @param bin ponteiro para o arquivo binário.
 * @return
 */
void escrever_cabecalho(RegistroCabecalho *c, FILE *bin) 
{
    fseek(bin, 0, SEEK_SET);                                     //move o ponteiro de escrita para o início do arquivo
    fwrite(&c->status, 1, 1, bin);                               //status
    fwrite(&c->RRNproxRegistro, 4, 1, bin);                      //próximo RRN
    fwrite(&c->numeroRegistrosInseridos, 4, 1, bin);             //num registros inseridos
    fwrite(&c->numeroRegistrosRemovidos, 4, 1, bin);             //num registros removidos
    fwrite(&c->numeroRegistrosAtualizados, 4, 1, bin);           //num registros atualizados
    fwrite(c->lixo, 1, 111, bin);                                //padding ($)
}


/**
 * Atualiza todos os campos do registro de cabeçalho. As mudanças não são escritas em disco.
 */
void atualizar_cabecalho(RegistroCabecalho *c, char status, int RRNproxRegistro, int numeroRegistrosInseridos, int numeroRegistrosAtualizados) 
{
    /* Atualiza valores do cabeçalho */
    c->status = status;
    c->RRNproxRegistro = RRNproxRegistro;
    c->numeroRegistrosInseridos = numeroRegistrosInseridos;
    c->numeroRegistrosAtualizados = numeroRegistrosAtualizados;
}


/**
 * Lê o cabeçalho do arquivo binário.
 * 
 * @param bin ponteiro para o arquivo binário.
 * @return ponteiro para o registro de cabeçalho caso o arquivo seja consistente ou NULL caso contrário.
 */
RegistroCabecalho *ler_cabecalho_bin(FILE *bin) {
    RegistroCabecalho *novoCabecalho = (RegistroCabecalho *) malloc(sizeof(RegistroCabecalho)); // Aloca o registro

    if((bin != NULL) && (novoCabecalho != NULL)) {
        /* Checa se o cabeçalho existe e está consistente */
        if((fread(&novoCabecalho->status, 1, 1, bin) == 1) && (novoCabecalho->status == '1')) { // Lê o campo status e verifica-o
            fread(&novoCabecalho->RRNproxRegistro, 4, 1, bin);                                  // Lê o campo RRNproxRegistro
            fread(&novoCabecalho->numeroRegistrosInseridos, 4, 1, bin);                         // Lê o campo numeroRegistrosInseridos
            fread(&novoCabecalho->numeroRegistrosRemovidos, 4, 1, bin);                         // Lê o campo numeroRegistrosRemovidos
            fread(&novoCabecalho->numeroRegistrosAtualizados, 4, 1, bin);                       // Lê o campo numeroRegistrosAtualizados
            fread(novoCabecalho->lixo, 1, 111, bin);                                            // Lê o padding do cabeçalho
            novoCabecalho->lixo[111] = '\0';

            return novoCabecalho;
        }
    }

    /* Apaga o cabeçalho alocado no caso do ponteiro de arquivo seja nulo */
    if(novoCabecalho != NULL) {
        free(novoCabecalho);
        novoCabecalho = NULL;
    }

    return NULL;
}


/**
 * Checa se há registros inseridos com base no cabeçalho.
 * 
 * @param cabecalho registro do cabeçalho.
 * @return retorna 1 se verdadeiro ou 0 se falso.
 */
int existe_registros(RegistroCabecalho *cabecalho) {
    return ((cabecalho != NULL) && (cabecalho->numeroRegistrosInseridos > 0)) ? 1 : 0;
}


/**
 * Lê o registro de dados apontado pelo ponteiro de escrita/leitura do arquivo binário.
 * 
 * @param bin ponteiro para o arquivo binário.
 * @return NULL caso o registro esteja logicamente removido; caso contrário, retorna o ponteiro para o registro de dados lido.
 */
RegistroPessoa* ler_registro_bin(FILE *bin) {
    int sizeCidadeMae, sizeCidadeBebe;                                              // Guarda os indicadores de tamanho dos campos variáveis
    RegistroPessoa *rp = (RegistroPessoa *) malloc(sizeof(RegistroPessoa));         // Registro de dados
    char lixoCampoVariavel[97];                                                     // Buffer para o espaço não utilizado dos campos variáveis
    
    if((bin != NULL) && (rp != NULL)) {
        /* Lê o indicador de tamanho de cidadeMae testando se o registro existe*/
        if((fread(&sizeCidadeMae, 4, 1, bin) == 1) && (sizeCidadeMae != -1)) {
            rp->cidadeMae = (char *) malloc((sizeCidadeMae + 1) * sizeof(char));    // Aloca memória para o campo variável

            /* Lê o indicador de tamanho de cidadeBebe */
            fread(&sizeCidadeBebe, 4, 1, bin);
            rp->cidadeBebe = (char *) malloc((sizeCidadeBebe + 1) * sizeof(char));  // Aloca memória para o campo variável

            /* Lê o campo cidadeMae */
            fread(rp->cidadeMae, 1, sizeCidadeMae, bin);
            rp->cidadeMae[sizeCidadeMae] = '\0';

            /* Lê o campo cidadeBebe */
            fread(rp->cidadeBebe, 1, sizeCidadeBebe, bin);
            rp->cidadeBebe[sizeCidadeBebe] = '\0';

            /* Lê o resto do espaço do campo variável */
            fread(lixoCampoVariavel, 1, (97 - (sizeCidadeMae + sizeCidadeBebe)), bin);
            
            /* Lê o campo idNascimento */
            fread(&rp->idNascimento, 4, 1, bin);
            
            /* Lê o campo idadeMae */
            fread(&rp->idadeMae, 4, 1, bin);
            
            /* Lê o campo dataNascimento */
            fread(rp->dataNascimento, 1, 10, bin);
            rp->dataNascimento[10] = '\0';
            
            /* Lê o campo sexoBebe */
            fread(&rp->sexoBebe, 1, 1, bin);
            
            /* Lê o campo estadoMae */
            fread(rp->estadoMae, 1, 2, bin);
            rp->estadoMae[2] = '\0';
            
            /* Lê o campo estadoBebe */
            fread(rp->estadoBebe, 1, 2, bin);
            rp->estadoBebe[2] = '\0';

            return rp;
        }
    }

    /* Apaga o registro alocado no caso do ponteiro de arquivo seja nulo */
    if(rp != NULL) {
        free(rp);
        rp = NULL;
    }

    return NULL;
}


/**
 * Imprime um registro de acordo com a formatação especificada na funcionalidade 2
 * @param rp Ponteiro para o registro de dados
 * @return
 */
void imprimir_registro_formatado(RegistroPessoa *rp) {
    printf("Nasceu em ");                       // Imprime "Nasceu em "
    imprimir_checar_vazio(rp->cidadeBebe);      // Imprime cidadeBebe ou "-"
    printf("/");                                // Imprime "/"
    imprimir_checar_vazio(rp->estadoBebe);      // Imprime estadoBebe ou "-"
    printf(", em ");                            // Imprime ", em "
    imprimir_checar_vazio(rp->dataNascimento);  // Imprime dataNascimento ou "-"
    printf(", um bebê de sexo ");               // Imprime ", um bebê de sexo "
    
    switch(rp->sexoBebe) {                      // Imprime "IGNORADO" se 0 ou nulo, MASCULINO se 1, FEMININO se 2
    case '0':
        printf("IGNORADO");
        break;
    case '1':
        printf("MASCULINO");
        break;
    case '2':
        printf("FEMININO");
        break;
    case '\0':
        printf("IGNORADO");
        break;
    }

    printf(".\n");      
}


/**
 * Checa se um campo do tipo string está vazio e imprime de acordo com a funcionalidade 2.
 * 
 * @param campo string do campo.
 * @return
 */
static void imprimir_checar_vazio(char *campo) {
    if(strcmp(campo, ""))         // Se a string não for vazia
        printf("%s", campo);      // Imprime o campo
    else                          // Se a string for vazia
        printf("-");              // Imprime "-"
}


/**
 * Imprime no STDOUT, de maneira formatada, as informações armazenadas em um registro. Função usada para testes.
 * 
 * @param rp ponteiro para o registro que será impresso.
 * @return 
 */
void imprimir_registro(RegistroPessoa *rp) {
    printf("< %s | %s | %d | %d | %s | %c | %s | %s >\n", 
        rp->cidadeMae, rp->cidadeBebe, rp->idNascimento, rp->idadeMae, rp->dataNascimento, rp->sexoBebe, rp->estadoMae, rp->estadoBebe);
}