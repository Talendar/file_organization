#include "registro_pessoa.h"
#include <stdlib.h>
#include <string.h>


static void fwrite_aux(char *campo, int n, FILE *bin);


/**
 * Definição da estrutura de um registro de uma pessoa, lido de um arquivo csv.
 */
struct RegistroPessoa {
     //TAMANHO VARIÁVEL
    char *cidadeMae,            //nome da cidade de residência da mãe
         *cidadeBebe;           //nome da cidade na qual o bebê nasceu

    //TAMANHO FIXO
    int  idNascimento,          //código sequencial que identifica univocamente cada registro do arquivo de dados
         idadeMae;              //idade da mãe do bebê
    char *dataNascimento,       //no formato AAAA-MM-DD
         *sexoBebe,             //pode assumir os valores ‘0’ (ignorado), ‘1’ (masculino) e ‘2’ (feminino)
         *estadoMae,            //sigla do estado da cidade de residência da mãe)
         *estadoBebe;           //sigla do estado da cidade na qual o bebê nasceu
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
        //to_do: strdup?
        rp->cidadeMae = strsep(&line, ",");
        rp->cidadeBebe = strsep(&line, ",");
        rp->idNascimento = atoi(strsep(&line, ","));
        rp->idadeMae = atoi(strsep(&line, ","));

        if(rp->idadeMae == 0)
            rp->idadeMae = -1;

        rp->dataNascimento = strsep(&line, ",");
        
        rp->sexoBebe = strsep(&line, ",");
        if(strlen(rp->sexoBebe) == 0) {
            rp->sexoBebe = malloc(sizeof(char));
            *(rp->sexoBebe) = '0';
        }

        rp->estadoMae = strsep(&line, ",");
        rp->estadoBebe = strsep(&line, ",");
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
    //to_do: "fix free(): invalid pointer"

    /*free((*rp)->cidadeMae);
    free((*rp)->cidadeBebe);
    free((*rp)->dataNascimento);
    free((*rp)->sexoBebe);
    free((*rp)->estadoMae);
    free((*rp)->estadoBebe);
    (*rp) = NULL;*/
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
    fwrite(rp->sexoBebe, 1, 1, bin);
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
 * Imprime no STDOUT, de maneira formatada, as informações armazenadas em um registro. Função usada para testes.
 * 
 * @param rp ponteiro para o registro que será impresso.
 * @return 
 */
void imprimir_registro(RegistroPessoa *rp) {
    printf("< %s | %s | %d | %d | %s | %s | %s | %s >\n", 
        rp->cidadeMae, rp->cidadeBebe, rp->idNascimento, rp->idadeMae, rp->dataNascimento, rp->sexoBebe, rp->estadoMae, rp->estadoBebe);
}