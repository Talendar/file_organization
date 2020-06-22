/**
 * Este módulo implementa o necessário para lidar com registros de dados em um arquivo.
 */


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
 * Cria um novo registro a partir dos dados especificados. O novo registro estará apenas na RAM, de forma que uma outra rotina será necessária caso se queira armazená-lo em um arquivo de disco. O nome dos parâmetros é auto-explicativo e refere-se aos respectivos campos do novo registro.
 * 
 * @param cidadeMae
 * @param cidadeBebe
 * @param idNascimento
 * @param idadeMae
 * @param dataNascimento
 * @param sexoBebe
 * @param estadoMae
 * @param estadoBebe
 * @return um ponteiro para o novo registro; retorna NULL caso haja falha na alocação dinâmica de memória.
 */
RegistroPessoa* criar_registro(char *cidadeMae, char *cidadeBebe, int idNascimento, int idadeMae, 
                                char dataNascimento[11], char sexoBebe[3], char estadoMae[3], char estadoBebe[3])
{
    RegistroPessoa *rp = malloc(sizeof(RegistroPessoa));
    if(rp != NULL) {
        //campos de tamanho variável
        rp->cidadeMae = malloc(strlen(cidadeMae) + 1);
        strcpy(rp->cidadeMae, cidadeMae);
        rp->cidadeBebe = malloc(strlen(cidadeBebe) + 1);
        strcpy(rp->cidadeBebe, cidadeBebe);


        //campos de tamanho fixo
        rp->idNascimento = idNascimento;
        rp->idadeMae = idadeMae;
        rp->sexoBebe = (!strcmp(sexoBebe, "-2")) ? -2 : sexoBebe[0];

        strcpy(rp->dataNascimento, dataNascimento);
        strcpy(rp->estadoMae, estadoMae);
        strcpy(rp->estadoBebe, estadoBebe);
    }

    return rp;
}


/**
 * Getter para o ID de nascimento do registro de dados.
 */
int registro_idNascimento(RegistroPessoa *rp) {
    return rp->idNascimento;
}


/**
 * Verifica se um dado registro candidato apresenta valores semelhantes, em um ou mais campos, a um registro modelo (pseudo-registro).
 * 
 * @param modelo pseudo-registro que servirá como modelo para a comparação; campos que não serão comparados devem possuir um valor default pré-definido (-1).
 * @param candidato registro que deseja-se comparar.
 * @return true caso o candidato tenha valores similares ao modelo ou false caso o contrário.
 */
bool registros_similares(RegistroPessoa *modelo, RegistroPessoa *candidato)
{
    if(
        (strcmp(modelo->cidadeMae, "-2") != 0 && strcmp(modelo->cidadeMae, candidato->cidadeMae) != 0) ||                     //checa cidadeMae
        (strcmp(modelo->cidadeBebe, "-2") != 0 && strcmp(modelo->cidadeBebe, candidato->cidadeBebe) != 0) ||                  //checa cidadeBebe
        (strcmp(modelo->dataNascimento, "-2") != 0 && strcmp(modelo->dataNascimento, candidato->dataNascimento) != 0) ||      //checa dataNascimento
        (strcmp(modelo->estadoMae, "-2") != 0 && strcmp(modelo->estadoMae, candidato->estadoMae) != 0) ||                     //checa estadoMae
        (strcmp(modelo->estadoBebe, "-2") != 0 && strcmp(modelo->estadoBebe, candidato->estadoBebe) != 0) ||                  //checa estadoBebe
        (modelo->idNascimento != -2 && modelo->idNascimento != candidato->idNascimento) ||                                    //checa idNascimento
        (modelo->idadeMae != -2 && modelo->idadeMae != candidato->idadeMae) ||                                                //checa idadeMae
        (modelo->sexoBebe != -2 && modelo->sexoBebe != candidato->sexoBebe)                                                   //checa sexoBebe
    )
        return false;

    return true;
}


/**
 * Remove logicamente o registro cujo fim é apontado pelo ponteiro do arquivo. Ao final da execução, o ponteiro do arquivo apontará para a posição final do registro logicamente removido. O registro de cabeçalho do arquivo não é atualizado.
 * 
 * @param bin arquivo binário já aberto (com permissão de escrita) e com ponteiro de posição apontando para o fim do registro a ser removido.
 * @return
 */
void remover_registro(FILE *bin) {
    const long int POS_INICIAL = ftell(bin);
    fseek(bin, POS_INICIAL - 128, SEEK_SET);    //recua o ponteiro do arquivo para o começo do registro

    int b = -1;  fwrite(&b, 4, 1, bin);         //marca o registro como logicamente removido  
    fseek(bin, POS_INICIAL, SEEK_SET);          //avança o ponteiro do arquivo para o final do registro logicamente removido
}


/**
 * Wrapper para a função "remover_registro". O parâmetro "rp" é ignorado.
 */
void remover_registro_aux(FILE *bin, RegistroPessoa *rp) {
    remover_registro(bin);
}


/**
 * Busca por registros com valores similares, em um ou mais campos, aos de um pseudo-registro modelo. Executa a função passada como parâmetro em cada um dos registros encontrados.
 * 
 * @param bin arquivo binário utilizado para a busca; o ponteiro do arquivo deve estar apontando para o início do primeiro registro.
 * @param modelo registro que servirá como modelo para a busca; campos que não serão considerados na busca devem possuir um valor default pré-definido (-1).
 * @param func ponteiro para a função que será executada nos registros encontrados.
 * @return a quantidade de registros encontrados.
 */
int buscar_registros(FILE *bin, RegistroPessoa *modelo, void (*func)(FILE *bin, RegistroPessoa *rp)) 
{   
    int count = 0;
    RegistroPessoa *rp = NULL;

     do {
        if(rp != NULL)
            liberar_registro(&rp, true);                //apaga da RAM o último registro lido

        //pula a leitura de registros logicamente removidos
        while(verificar_removido(bin)) 
            pular_registro(bin);

        //lê o próximo registro, verifica se EOF e imprime o registro
        if((rp = ler_registro_bin(bin)) != NULL) {
            if(registros_similares(modelo, rp)) {       //verifica se o registro satisfaz os critérios de busca
                func(bin, rp);                          //executa a função passada como argumento
                count++;
            }
        }
    } while(!fim_do_arquivo(bin));

    if(rp != NULL)
        liberar_registro(&rp, true);                    // Apaga o último registro

    return count;     //retorna a quantidade de registros encontrados
}


/**
 * Busca, em um arquivo, pelo registro com o RRN especificado.
 * 
 * @param rrn RRN (relative record number) do registro que se deseja resgatar.
 * @param bin arquivo binário no qual será realizada a busca.
 * @return o registro, caso encontrado, ou NULL caso ele tenha sido removido ou o rrn for inválido.
 */
RegistroPessoa* registro_em(int rrn, FILE *bin) {
    fseek(bin, 0, SEEK_END);
    const int MAX = ftell(bin);

    int offset = (rrn + 1) * 128;                                          //calcula o offset do registro
    fseek(bin, offset, SEEK_SET);                                          //move o ponteiro do arquivo para o offset equivalente ao rrn especificado

    if(offset < 128 || offset > (MAX - 128) || verificar_removido(bin))    //verifica se o offset é valido e se o registro não foi removido
        return NULL;

    return ler_registro_bin(bin);                                          //lê e retorna o registro no offset
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
        c->status = '0';            // arquivo marcado, inicialmente, como inconsistente
        c->RRNproxRegistro = 0;
        c->numeroRegistrosInseridos = 0;
        c->numeroRegistrosAtualizados = 0;
        c->numeroRegistrosRemovidos = 0;

        /* Preenche com '$' o lixo */
        for(int i = 0; i < 111; i++)
            c->lixo[i] = '$';
        c->lixo[111] = '\0';        // Coloca o '\0' no fim para usar como string
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
 * Atualiza todos os campos do registro de cabeçalho. As mudanças não são escritas em disco. Campos que devem manter o seu valor devem receber o valor -1 no argumento.
 */
void atualizar_cabecalho(RegistroCabecalho *c, char status, int RRNproxRegistro, int numeroRegistrosInseridos, int numeroRegistrosRemovidos, int numeroRegistrosAtualizados) 
{
    /* Atualiza valores do cabeçalho */
    c->status = (status == -1) ? c->status : status;
    c->RRNproxRegistro = (RRNproxRegistro == -1) ? c->RRNproxRegistro : RRNproxRegistro;
    c->numeroRegistrosInseridos = (numeroRegistrosInseridos == -1) ? c->numeroRegistrosInseridos : numeroRegistrosInseridos;
    c->numeroRegistrosRemovidos = (numeroRegistrosRemovidos == -1) ? c->numeroRegistrosRemovidos : numeroRegistrosRemovidos;
    c->numeroRegistrosAtualizados = (numeroRegistrosAtualizados == -1) ? c->numeroRegistrosAtualizados : numeroRegistrosAtualizados;
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
 * Verifica se o próximo registro a ser lido do arquivo binário consta como logicamente removido.
 * 
 * @param bin arquivo binário já aberto.
 * @return true caso o registro esteja logicamente removido ou false caso contrário (incluindo se o arquivo estiver em EOF).
 */
bool verificar_removido(FILE *bin) {
    int sizeCidadeMae = 0;
    const long int pos_inicial = ftell(bin);

    fread(&sizeCidadeMae, 4, 1, bin);       //lê o primeiro campo do registro
    fseek(bin, pos_inicial, SEEK_SET);      //retorna o ponteiro do arquivo para sua posição inicial

    return (sizeCidadeMae == -1);
}


/**
 * Aponta o ponteiro do arquivo binário para o início do próximo arquivo a ser lido, "pulando" a leitura do registro atual.
 * 
 * @param bin arquivo binário já aberto.
 * @return
 */
void pular_registro(FILE *bin) {
    fseek(bin, +128, SEEK_CUR);
}

/**
 * Atualiza os campos de um registro com base em outro.
 * 
 * @param original registro a ser modificado.
 * @param alteracoes registro com campos a serem modificados.
 * @return
 */
void atualizar_registro(RegistroPessoa *original, RegistroPessoa *alteracoes) {
    if(original == NULL || alteracoes == NULL) return;

    if(strncmp(alteracoes->cidadeMae, "-2", 3) != 0) {
        free(original->cidadeMae);
        original->cidadeMae = NULL;
        original->cidadeMae = (char *) malloc((strlen(alteracoes->cidadeMae)+1) * sizeof(char));
        if(original->cidadeMae != NULL)
            strcpy(original->cidadeMae, alteracoes->cidadeMae);
    }
    
    if(strncmp(alteracoes->cidadeBebe, "-2", 3) != 0) {
        free(original->cidadeBebe);
        original->cidadeBebe = NULL;
        original->cidadeBebe = (char *) malloc((strlen(alteracoes->cidadeBebe)+1) * sizeof(char));
        if(original->cidadeMae != NULL)
            strcpy(original->cidadeBebe, alteracoes->cidadeBebe);
    }
    if(alteracoes->idNascimento != -2)
        original->idNascimento = alteracoes->idNascimento;
    
    if(alteracoes->idadeMae != -2)
        original->idadeMae = alteracoes->idadeMae;
    
    if(strncmp(alteracoes->dataNascimento, "-2", 3) != 0)
        strcpy(original->dataNascimento, alteracoes->dataNascimento);
    
    if(alteracoes->sexoBebe != -2)
        original->sexoBebe = alteracoes->sexoBebe;
    
    if(strncmp(alteracoes->estadoMae, "-2", 3) != 0)
        strcpy(original->estadoMae, alteracoes->estadoMae);
    
    if(strncmp(alteracoes->estadoBebe, "-2", 3) != 0)
        strcpy(original->estadoBebe, alteracoes->estadoBebe);
}


/**
 * Atualiza o registro no arquivo mantendo o lixo já presente.
 * 
 * @param rp registro contendo os campos atualizados.
 * @param bin arquivo binário a ser escrito.
 * @param rrn RRN do registro a ser atualizado.
 * @return
 */
void atualizar_mantendo_lixo(RegistroPessoa *rp, FILE *bin, int rrn) {
    if(rp == NULL || bin == NULL) return;

    int tamCidadeMae = strlen(rp->cidadeMae);
    int tamCidadeBebe = strlen(rp->cidadeBebe);

    /* Testa se o registro original existe */
    RegistroPessoa *teste = registro_em(rrn, bin);  // Busca o registro original
    if(teste == NULL) return;                       // Encerra a execução se não existe
    liberar_registro(&teste, true);                 // Apaga o teste
    
    /* Atualiza o registro */
    fseek(bin, ((long) rrn * 128) + 128, SEEK_SET); // Procura a posição do registro
    fwrite(&tamCidadeMae, 4, 1, bin);
    fwrite(&tamCidadeBebe, 4, 1, bin);
    fwrite(rp->cidadeMae, 1, tamCidadeMae, bin);
    fwrite(rp->cidadeBebe, 1, tamCidadeBebe, bin);
    fseek(bin, (97 - (tamCidadeMae + tamCidadeBebe)), SEEK_CUR);
    fwrite(&rp->idNascimento, 4, 1, bin);
    fwrite(&rp->idadeMae, 4, 1, bin);
    fwrite_aux(rp->dataNascimento, 10, bin);
    fwrite(&rp->sexoBebe, 1, 1, bin);
    fwrite_aux(rp->estadoMae, 2, bin);
    fwrite_aux(rp->estadoBebe, 2, bin);
}


/**
 * Verifica se o ponteiro do arquivo está apontando para a última posição do arquivo (fim do arquivo).
 * 
 * @param bin arquivo a ser analizado.
 * @return true caso o arquivo esteja no fim ou false no caso contrário.
 */
bool fim_do_arquivo(FILE *bin) {
    long int current = ftell(bin);
    fseek(bin, 0, SEEK_END);
    
    long int end = ftell(bin);
    fseek(bin, current, SEEK_SET);

    return current == end;
}


/**
 * Lê o registro de dados apontado pelo ponteiro de escrita/leitura do arquivo binário.
 * 
 * @param bin ponteiro para o arquivo binário.
 * @return NULL caso o registro esteja logicamente removido; caso contrário, retorna o ponteiro para o registro de dados lido.
 */
RegistroPessoa* ler_registro_bin(FILE *bin) 
{
    int sizeCidadeMae, sizeCidadeBebe;                                              // Guarda os indicadores de tamanho dos campos variáveis
    RegistroPessoa *rp = (RegistroPessoa *) malloc(sizeof(RegistroPessoa));         // Registro de dados
    char lixoCampoVariavel[97];                                                     // Buffer para o espaço não utilizado dos campos variáveis
    
    if((bin != NULL) && (rp != NULL) && !fim_do_arquivo(bin)) {
        /* Lê o indicador de tamanho de cidadeMae testando se o registro existe*/

        fread(&sizeCidadeMae, 4, 1, bin);
        rp->cidadeMae = (char *) malloc((sizeCidadeMae + 1) * sizeof(char));        // Aloca memória para o campo variável

        /* Lê o indicador de tamanho de cidadeBebe */
        fread(&sizeCidadeBebe, 4, 1, bin);
        rp->cidadeBebe = (char *) malloc((sizeCidadeBebe + 1) * sizeof(char));      // Aloca memória para o campo variável

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

    /* Apaga o registro alocado no caso do ponteiro de arquivo seja nulo */
    if(rp != NULL) {
        free(rp);
        rp = NULL;
    }

    return NULL;
}


/**
 * Retorna o RRN do próximo registro a ser inserido.
 */
int proximo_rrn(RegistroCabecalho *cabecalho) {
    return cabecalho->RRNproxRegistro;
}


/**
 * Retorna a quantidade de registros inseridos em um arquivo a partir de seu cabeçalho.
 * 
 * @param c cabeçalho do arquivo.
 * @return quantidade de registros inseridos no arquivo até o momento.
 */
int qnt_registros_inseridos(RegistroCabecalho *c) {
    return c->numeroRegistrosInseridos;
}


/**
 * Retorna a quantidade de registros removidos de um arquivo a partir de seu cabeçalho.
 * 
 * @param c cabeçalho do arquivo.
 * @return quantidade de registros removidos do arquivo até o momento.
 */
int qnt_registros_removidos(RegistroCabecalho *c) {
    return c->numeroRegistrosRemovidos;
}


/**
 * Retorna a quantidade de registros atualizados de um arquivo a partir de seu cabeçalho.
 * 
 * @param c cabeçalho do arquivo.
 * @return quantidade de registros atualizados do arquivo até o momento.
 */
int qnt_registros_atualizados(RegistroCabecalho *c) {
    return c->numeroRegistrosAtualizados;
}


/**
 * Imprime um registro de acordo com a formatação especificada na funcionalidade 2
 * @param rp Ponteiro para o registro de dados
 * @return
 */
void imprimir_registro_formatado(RegistroPessoa *rp) {
    printf("Nasceu em ");                       // Imprime "Nasceu em "
    imprimir_checar_vazio(rp->cidadeBebe);      // Imprime cidadeBebe ou "-"
    printf("/");                                
    imprimir_checar_vazio(rp->estadoBebe);      // Imprime estadoBebe ou "-"
    printf(", em ");                            
    imprimir_checar_vazio(rp->dataNascimento);  // Imprime dataNascimento ou "-"
    printf(", um bebê de sexo ");               
    
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
void imprimir_registro_teste(RegistroPessoa *rp) {
    printf("< %s | %s | %d | %d | %s | %c | %s | %s >\n", 
        rp->cidadeMae, rp->cidadeBebe, rp->idNascimento, rp->idadeMae, rp->dataNascimento, rp->sexoBebe, rp->estadoMae, rp->estadoBebe);
}


/**
 * Wrapper para a função "imprimir_registro_formatado".
 */
void imprimir_registro_aux(FILE *bin, RegistroPessoa *rp) {
    imprimir_registro_formatado(rp);
}


/**
 * Imprime no STDOUT, de maneira formatada, as informações armazenadas em um cabeçalho. Função usada para testes.
 * 
 * @param c ponteiro para o cabeçalho que será impresso.
 * @return 
 */
void imprimir_cabecalho_teste(RegistroCabecalho *c) {
    printf("< %c | %d | %d | %d | %d | %s >\n",
        c->status, c->RRNproxRegistro, c->numeroRegistrosInseridos, c->numeroRegistrosRemovidos, c->numeroRegistrosAtualizados, c->lixo);
}