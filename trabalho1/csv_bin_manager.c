#include "csv_bin_manager.h"
#include "registro_pessoa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Cria um arquivo binário a partir de um arquivo csv.
 * 
 * @param csv_pathname pathname para o arquivo csv que será lido.
 * @param bin_pathname pathname para o arquivo binário que será gerado.
 * @return retorna true caso a conversão tenha ocorrido com sucesso e false no caso contrário.
 */
bool csv_para_binario(char *csv_pathname, char *bin_pathname) 
{   
    FILE *csv, *bin;                                      //ponteiros para os arquivos csv e binário
    if((csv = fopen(csv_pathname, "r")) == NULL)   
        return false;
    if((bin = fopen(bin_pathname, "wb")) == NULL) {
        fclose(csv);
        return false;
    }

    //criando header
    int zero = 0;  
    fwrite(&zero, 1, 1, bin);                             //status
    fwrite(&zero, 4, 1, bin);                             //próximo RRN
    fwrite(&zero, 4, 1, bin);                             //num registros inseridos
    fwrite(&zero, 4, 1, bin);                             //num registros removidos
    fwrite(&zero, 4, 1, bin);                             //num registros atualizados
    
    char pad_char = '$';
    for(int i = 0; i < 111; i++)
        fwrite(&pad_char, 1, 1, bin);                     //padding ($)

    //lendo csv e escrevendo binário
    char *line;                                           //linha a ser lida
    size_t len = 0;                                       //tamanho da linha lida
    int count = 0;                                        //num registros lidos
    getline(&line, &len, csv);                            //consome a primeira linha (header)
    
    while(getline(&line, &len, csv) != -1) 
    {
        trim(line);                                       //removendo \n no final
        RegistroPessoa *registro = ler_registro(line);
        if(registro == NULL)
            return false;

        registro2bin(registro, bin);                      //inserindo o registro no arquivo binário
        count++;
        //printf("%ld\n", ftell(bin));
        liberar_registro(&registro);
    }

    //atualizando header
    fseek(bin, 0, 0);
    char one = '1';  fwrite(&one, 1, 1, bin);             //status = 1

    fseek(bin, 1, 0);
    fwrite(&count, 4, 1, bin);                            //campo rrn próximo

    fseek(bin, 5, 0);                                     
    fwrite(&count, 4, 1, bin);                            //campo número registros inseridos

    //finalizando
    fclose(csv);
    fclose(bin);
    return true;
}


/**
 * Lê o arquivo binário e imprime (no STDOUT) algumas informações relativas a cada um de seus registros.
 * 
 * @param bin_pathname pathname para o arquivo binário que será lido.
 * @return true caso o procedimento tenha executado corretamente; false caso tenha havido algum erro.
 */
bool bin2txt(char *bin_pathname) 
{
    RegistroPessoa *rp = NULL;
    RegistroCabecalho *cabecalho = NULL;
    FILE* bin = fopen(bin_pathname, "rb");
    
    if((bin != NULL) && ((cabecalho = ler_cabecalho(bin)) != NULL)) {
        if(existeRegistros(cabecalho)) {
            while((rp = ler_registro_bin(bin)) != NULL) {
                imprimir_registro_formatado(rp);
                free(rp);
                rp = NULL;
            }
            
            free(cabecalho);
            cabecalho = NULL;

            fclose(bin);
            return true;
        } else {
            printf("Registro inexistente.");
            free(cabecalho);
            cabecalho = NULL;
        }
    } else {
        printf("Falha no processamento do arquivo.\n");
    }

    return false;
}


/** 
 * Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente. Ela vai abrir de novo para leitura e depois fechar.
 */
void binarioNaTela(char *nomeArquivoBinario) 
{
	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}


/**
 *	Essa função arruma uma string de entrada "str".
 *	Manda pra ela uma string que tem '\r' e ela retorna sem.
 *	Ela remove do início e do fim da string todo tipo de espaçamento (\r, \n, \t, espaço, ...).
 *	Por exemplo:
 *
 *	char minhaString[] = "    \t TESTE  DE STRING COM BARRA R     \t  \r\n ";
 *	trim(minhaString);
 *	printf("[%s]", minhaString); // vai imprimir "[TESTE  DE STRING COM BARRA R]"
 *
 */
void trim(char *str) {
	size_t len;
	char *p;

	// remove espaçamentos do fim
	for(len = strlen(str); len > 0 && isspace(str[len - 1]); len--);
	str[len] = '\0';
	
	// remove espaçamentos do começo
	for(p = str; *p != '\0' && isspace(*p); p++);
	len = strlen(p);
	
	memmove(str, p, sizeof(char) * (len + 1));
}

