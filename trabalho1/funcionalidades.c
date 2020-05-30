#include "funcionalidades.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "csv_bin_manager.h"
#include "registro_pessoa.h"


/**
 * Funcionalidade 1 do programa. Cria um arquivo binário a partir dos dados de um arquivo CSV.
 * 
 * @param csv_pathname nome do arquivo csv.
 * @param bin_pathname nome do arquivo binário que será criado.
 */
void func1(char *csv_pathname, char *bin_pathname) {
    if(csv_para_binario(csv_pathname, bin_pathname))    // Executa a funcionalidade e checa se o ela ocorreu conforme esperado
        binarioNaTela(bin_pathname);                    // Imprime na tela
    else 
        printf("Falha no carregamento do arquivo.\n");  // Msg de erro
}


/**
 * Funcionalidade 2 do programa. Imprime, de forma formatada, os dados do arquivo binário.
 * 
 * @param bin arquivo binário a ser utilizado.
 */
void func2(FILE *bin) {
    bin2txt(bin);
}


/*
*	Use essa função para ler um campo string delimitado entre aspas (").
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
static void scan_quote_string(char *str) {
	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}


/**
 * Lê, do STDOUT, campos de interesse de um registro a partir de uma entrada do tipo:
 *      m1 nomeCampo11 valorCampo11 [nomeCampo12 valorCampo12 [...
 * e cria um pseudo-registro (armazenado apenas na RAM e não em disco) com os valores obtidos.
 * 
 * @return um ponteiro para o registro criado.
 */
static RegistroPessoa* ler_campos() {
     int m;  
     scanf(" %d", &m);              //lê a quantidade de campos de busca 

    //declarando variáveis dos campos com valor padrão (indica que não foram lidas)
    int idNascimento = -1, idadeMae = -1;
    char cidadeMae[128] = "-1", cidadeBebe[128] = "-1", 
         dataNascimento[13] = "-1", sexoBebe[4] = "-1", 
         estadoMae[5] = "-1", estadoBebe[5] = "-1";

    //lendo os campos
    for(int i = 0; i < m; i++) {
        char campo[32];  scanf(" %s", campo);     //lê o nome do campo
        
        if(!strcmp(campo, "idNascimento")) 
            scanf(" %d", &idNascimento);          //lê o campo idNascimento
        else if(!strcmp(campo, "idadeMae")) 
            scanf(" %d", &idadeMae);              //lê o campo idadeMae
        else if(!strcmp(campo, "cidadeMae")) 
            scan_quote_string(cidadeMae);         //lê o campo cidadeMae
        else if(!strcmp(campo, "cidadeBebe")) 
            scan_quote_string(cidadeBebe);        //lê o campo cidadeBebe
        else if(!strcmp(campo, "dataNascimento")) 
            scan_quote_string(dataNascimento);    //lê o campo dataNascimento
        else if(!strcmp(campo, "sexoBebe")) 
            scan_quote_string(sexoBebe);         //lê o campo sexoBebe
        else if(!strcmp(campo, "estadoMae")) 
            scan_quote_string(estadoMae);         //lê o campo cidadeBebe
        else if(!strcmp(campo, "estadoBebe")) 
            scan_quote_string(estadoBebe);        //lê o campo estadoBebe
    }

    //retornando pseudo-registro
    return criar_registro(cidadeMae, cidadeBebe, idNascimento, idadeMae, dataNascimento, sexoBebe, estadoMae, estadoBebe);
}


/**
 * Funcionalidade 3 do programa. Busca por registros com as características fornecidas e os imprime.
 * 
 * @param bin arquivo binário a ser utilizado.
 */
void func3(FILE *bin) 
{
    fseek(bin, 128, SEEK_SET);                  //avança o ponteiro do arquivo para o início do primeiro registro (pula o cabeçalho)
    int encontrados = 0;

    RegistroPessoa *wanted = ler_campos();      //recebe um pseudo-registro com valores dos parâmetros de busca
    encontrados = buscar_registros(bin, wanted, &imprimir_registro_aux);
    liberar_registro(&wanted, true);
    
    if(encontrados == 0)
        printf("Registro Inexistente.\n");
}


/**
 * Funcionalidade 4 do programa. Lê um RRN do usuário e imprime o registro correspondente no arquivo binário.
 * 
 * @param bin arquivo binário a ser utilizado.
 */
void func4(FILE *bin) 
{
    int rrn;  scanf(" %d", &rrn);        //lê o rrn do registro a ser buscado
    RegistroPessoa *rp = registro_em(rrn, bin);

    if(rp != NULL) {
        imprimir_registro_formatado(rp);
        liberar_registro(&rp, true);
    }
    else 
        printf("Registro Inexistente.\n");
}


/**
 * Funcionalidade 5 do programa. Especificadas características, remove os registros correspondentes do arquivo. Durante a execução da função o arquivo binário será fechado (fclose).
 * 
 * @param bin_pathname nome do arquivo binário; necessário para a chamada da função binarioNaTela.
 * @param bin ponteiro para o arquivo binário a ser utilizado.
 */
void func5(char *bin_pathname, FILE *bin) 
{
    int n;  scanf(" %d", &n);                                           //lê número de remoções
    RegistroCabecalho *cabecalho = ler_cabecalho_bin(bin);

    atualizar_cabecalho(cabecalho, '0', -1, -1, -1, -1);                //marca no cabeçalho que o arquivo está inconsistente
    int count = qnt_registros_removidos(cabecalho);

    for(int i = 0; i < n; i++) {
        RegistroPessoa *wanted = ler_campos();                          //recebe um pseudo-registro com valores dos parâmetros de busca
        fseek(bin, 128, SEEK_SET);                                      //aponta o ponteiro do arquivo para o primeiro registro
        count += buscar_registros(bin, wanted, &remover_registro_aux);  //busca e remove os registros encontrados
        liberar_registro(&wanted, true);
    }

    atualizar_cabecalho(cabecalho, '1', -1, 
        qnt_registros_inseridos(cabecalho) - count, count, -1);         //atualiza no cabeçalho os campos "status", "numeroRegistrosInseridos" e "numeroRegistrosRemovidos"
    escrever_cabecalho(cabecalho, bin);                                 //escreve o cabeçalho atualizado no arquivo

    fclose(bin);                                                        //necessário para a execução da binarioNaTela
    binarioNaTela(bin_pathname);
}


/**
 * Funcionalidade 6 do programa. TO_DO: descrição
 * 
 * @param bin arquivo binário a ser utilizado.
 */
void func6(FILE *bin) {
    //to_do
}


/**
 * Funcionalidade 7 do programa. TO_DO: descrição
 * 
 * @param bin arquivo binário a ser utilizado.
 */
void func7(FILE *bin) {
    //to_do
}