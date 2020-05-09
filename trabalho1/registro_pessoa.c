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
RegistroPessoa* ler_registro(char *line) 
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
 * @return
 */
void liberar_registro(RegistroPessoa **rp) {
    //a memória alocada pelos campos de tamanho variável de rp já foi liberada (peculiaridade do uso da função strsep)
    free(*rp);
    (*rp) = NULL;
}


/**
 * Grava o registro no final do arquivo binário fornecido.
 * 
 * @param rp ponteiro para um registro.
 * @param bin ponteiro para o arquivo binário.
 * @return
 */
void registro2bin(RegistroPessoa *rp, FILE *bin) 
{
    int sizeCidadeMae = strlen(rp->cidadeMae), sizeCidadeBebe = strlen(rp->cidadeBebe);
    fwrite(&sizeCidadeMae, 4, 1, bin);   //tamanho cidade mae
    fwrite(&sizeCidadeBebe, 4, 1, bin);  //tamanho cidade bebe

    fwrite(rp->cidadeMae, 1, strlen(rp->cidadeMae), bin);
    fwrite(rp->cidadeBebe, 1, strlen(rp->cidadeBebe), bin);

    int pad = 105 - (8 + strlen(rp->cidadeMae) + strlen(rp->cidadeBebe));
    char pad_char = '$';
    for(int i = 0; i < pad; i++)
        fwrite(&pad_char, 1, 1, bin);    //adicionando padding ($)

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
        fwrite(campo, 1, n, bin);
    else {
        char null_char = '\0', pad_char = '$';
        fwrite(&null_char, 1, 1, bin);
        for(int i = 1; i < n; i++) 
            fwrite(&pad_char, 1, 1, bin);
    }
}


/**
 * Cria um novo registro de cabeçalho para o arquivo.
 * 
 * @return um ponteiro para o novo cabeçalho.
 */
RegistroCabecalho* criar_cabecalho() 
{
    RegistroCabecalho *c = malloc(sizeof(RegistroCabecalho));
    if(c != NULL) {
        c->status = '0';
        c->RRNproxRegistro = 0;
        c->numeroRegistrosInseridos = 0;
        c->numeroRegistrosAtualizados = 0;

        for(int i = 0; i < sizeof(c->lixo) - 1; i++)
            c->lixo[i] = '$';
        c->lixo[sizeof(c->lixo) - 1] = '\0';
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
    fseek(bin, 0, 0);                                            //move o ponteiro de escrita para o início do arquivo
    fwrite(&c->status, 1, 1, bin);                               //status
    fwrite(&c->RRNproxRegistro, 4, 1, bin);                      //próximo RRN
    fwrite(&c->numeroRegistrosInseridos, 4, 1, bin);             //num registros inseridos
    fwrite(&c->numeroRegistrosRemovidos, 4, 1, bin);             //num registros removidos
    fwrite(&c->numeroRegistrosAtualizados, 4, 1, bin);           //num registros atualizados

    for(int i = 0; i < strlen(c->lixo); i++)
        fwrite(c->lixo + i, 1, 1, bin);                          //padding ($)
}


/**
 * Atualiza todos os campos do registro de cabeçalho. As mudanças não são escritas em disco.
 */
void atualizar_cabecalho(RegistroCabecalho *c, char status, int RRNproxRegistro, int numeroRegistrosInseridos, int numeroRegistrosAtualizados) 
{
    c->status = status;
    c->RRNproxRegistro = RRNproxRegistro;
    c->numeroRegistrosInseridos = numeroRegistrosInseridos;
    c->numeroRegistrosAtualizados = numeroRegistrosAtualizados;
}


/**
 * TO DO.
 */
RegistroCabecalho *ler_cabecalho_bin(FILE *bin) {
    RegistroCabecalho *novoCabecalho = (RegistroCabecalho *) malloc(sizeof(RegistroCabecalho));

    if((bin != NULL) && (novoCabecalho != NULL)) {
        if((fread(&novoCabecalho->status, 1, 1, bin) == 1) && (novoCabecalho->status == 1)) {
            fread(&novoCabecalho->RRNproxRegistro, 4, 1, bin);
            fread(&novoCabecalho->numeroRegistrosInseridos, 4, 1, bin);
            fread(&novoCabecalho->numeroRegistrosRemovidos, 4, 1, bin);
            fread(&novoCabecalho->numeroRegistrosAtualizados, 4, 1, bin);
            fread(novoCabecalho->lixo, 1, 111, bin);

            return novoCabecalho;
        }
    }

    return NULL;
}


/**
 * TO DO.
 */
int existeRegistros(RegistroCabecalho *cabecalho) {
    return ((cabecalho != NULL) && (cabecalho->numeroRegistrosInseridos > 0)) ? 1 : 0;
}


/**
 * TO DO.
 */
RegistroPessoa *ler_registro_bin(FILE *bin) {
    int sizeCidadeMae, sizeCidadeBebe;
    char existeRegistro;
    RegistroPessoa *rp = malloc(sizeof(RegistroPessoa));

    if((bin != NULL) && (rp != NULL)) {
        if((fread(&existeRegistro, 1, 1, bin) == 1) && (existeRegistro != '*')) {
            fseek(bin, -1, SEEK_CUR);

            fread(&sizeCidadeMae, 4, 1, bin);

            fread(&sizeCidadeBebe, 4, 1, bin);
            
            fread(rp->cidadeMae, 1, sizeCidadeMae, bin);
            rp->cidadeMae[sizeCidadeMae] = '\0';
            
            fread(rp->cidadeBebe, 1, sizeCidadeBebe, bin);
            rp->cidadeBebe[sizeCidadeBebe] = '\0';

            fread(&rp->idNascimento, 4, 1, bin);
            
            fread(&rp->idadeMae, 4, 1, bin);
            
            fread(rp->dataNascimento, 1, 10, bin);
            rp->dataNascimento[10] = '\0';
            
            fread(&rp->sexoBebe, 1, 1, bin);
            
            fread(rp->estadoMae, 1, 2, bin);
            rp->estadoMae[2] = '\0';
            
            fread(rp->estadoBebe, 1, 2, bin);
            rp->estadoBebe[2] = '\0';
        
            return rp;
        }
    }

    return NULL;
}


/**
 * TO DO.
 */
void imprimir_registro_formatado(RegistroPessoa *rp) {
    printf("Nasceu em ");
    imprimir_checar_vazio(rp->cidadeBebe);
    printf("/");
    imprimir_checar_vazio(rp->estadoBebe);
    printf(", em ");
    imprimir_checar_vazio(rp->dataNascimento);
    printf(", um bebe de sexo ");
    
    switch(rp->sexoBebe) {
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
 * TO DO.
 */
static void imprimir_checar_vazio(char *campo) {
    if(strcmp(campo, ""))
        printf("%s", campo);
    else
        printf("-");
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