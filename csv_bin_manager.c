/**
 * Este módulo lida com a leitura e escrita de arquivos binários e csv.
 */


#include "csv_bin_manager.h"
#include "registro_pessoa.h"
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
    FILE *csv = NULL, *bin = NULL;                        //ponteiros para os arquivos csv e binário
    if((csv = fopen(csv_pathname, "r")) == NULL)   
        return false;
    if((bin = fopen(bin_pathname, "wb")) == NULL) {
        fclose(csv);
        return false;
    }

    //criando header
    RegistroCabecalho *cabecalho = criar_cabecalho();
    escrever_cabecalho(cabecalho, bin);

    //lendo csv e escrevendo binário
    char *line = NULL;                                    //linha a ser lida
    size_t len = 0;                                       //tamanho da linha lida
    int count = 0;                                        //num registros lidos
    
    getline(&line, &len, csv);                            //consome a primeira linha (header)
    free(line);  line = NULL;

    while(getline(&line, &len, csv) != -1)                //a função getline retorna -1 no EOF
    {
        trim(line);                                       //removendo \n no final
        char *line2free = strdup(line);                   //necessário para liberar a memória alocada, devido ao uso da função strsep

        RegistroPessoa *registro = ler_registro_csv(line);
        if(registro == NULL)
            return false;                                 //retorna false caso não tenha sido possível ler o registro

        registro2bin(registro, bin);                      //inserindo o registro no arquivo binário
        count++;

        //liberando a memória alocada
        free(line2free);
        liberar_registro(&registro, false);
        free(line);  line = NULL;
    }

    free(line);
    line = NULL;

    //atualizando header
    atualizar_cabecalho(cabecalho, '1', count, count, 0, 0);
    escrever_cabecalho(cabecalho, bin);
    free(cabecalho);

    //finalizando
    fclose(csv);
    fclose(bin);
    return true;
}


/**
 * Lê o arquivo binário e imprime (no STDOUT) algumas informações relativas a cada um de seus registros.
 * 
 * @param bin arquivo binário que será lido.
 * @return true caso o procedimento tenha executado corretamente; false caso tenha havido algum erro.
 */
bool bin2txt(FILE *bin) 
{
    RegistroCabecalho *cabecalho = NULL;    // Registro de cabeçalho
    
    /* Checa se o cabeçalho existe e se o arquivo é consistente */
    if(((cabecalho = ler_cabecalho_bin(bin)) != NULL)) {
        if(existe_registros(cabecalho)) {                                                         // Checa se há registros de dados
            RegistroPessoa *modelo = criar_registro("-2", "-2", -2, -2, "-2", "-2", "-2", "-2");  // Registro modelo de busca com valores defaults (indicam que o campo deve ser ignorado na busca)
            buscar_registros(bin, modelo, &imprimir_registro_aux);                                // Imprime todos os registros do arquivo
            
            free(cabecalho);    // Apaga o cabeçalho da memória RAM
            cabecalho = NULL;   // Restaura o ponteiro para NULL

            return true;        // Retorna com sucesso
        } else {
            printf("Registro inexistente.");    // Imprime mensagem de registro de dados inexistente
            free(cabecalho);                    // Apaga o cabeçalho da memória ram
            cabecalho = NULL;                   // Restaura o ponteiro para NULL
        }
    } 
    else 
        printf("Falha no processamento do arquivo.");

    return false;       // Retorna com erros
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

/*
*	Use esta função para ler um campo string delimitado entre aspas (").
*	Chame ela na hora que for ler tal campo. Por exemplo:
*
*	A entrada está da seguinte forma:
*		nomeDoCampo "MARIA DA SILVA"
*
*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
*
*/
void scan_quote_string(char *str) {
	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') {                   // campo NULO
		getchar(); getchar(); getchar();         // ignorar o "ULO" de NULO.
		strcpy(str, "");                         // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) {          // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar();        // ignorar aspas fechando
	} else if(R != EOF){  // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		str[0] = R;
		scanf("%s", &str[1]);
	} else {            // EOF
		strcpy(str, "");
	}
}