/**
 * Este módulo tem por objetivo encapsular a execução das funcionalidades do programa.
 */


#include "funcionalidades.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "csv_bin_manager.h"
#include "registro_pessoa.h"
#include "arvore_b.h"


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
    int idNascimento = -2, idadeMae = -2;
    char cidadeMae[128] = "-2", cidadeBebe[128] = "-2", 
         dataNascimento[13] = "-2", sexoBebe[4] = "-2", 
         estadoMae[5] = "-2", estadoBebe[5] = "-2",
         buffer[12];
    
    //lendo os campos
    for(int i = 0; i < m; i++) {
        char campo[32];  scanf(" %s", campo);     //lê o nome do campo
        
        if(!strcmp(campo, "idNascimento")) {
            scan_quote_string(buffer);            //lê o campo idNascimento
            if(strcmp(buffer, "") == 0)           //converte para inteiro
                idNascimento = -1;                //recebe NULO
            else
                idNascimento = atoi(buffer);      //recebe inteiro
        }
        else if(!strcmp(campo, "idadeMae")) {
            scan_quote_string(buffer);            //lê o campo idadeMae
            if(strcmp(buffer, "") == 0)           //converte para inteiro
                idadeMae = -1;                    //recebe NULO
            else
                idadeMae = atoi(buffer);          //recebe inteiro
        }
        else if(!strcmp(campo, "cidadeMae")) 
            scan_quote_string(cidadeMae);         //lê o campo cidadeMae
        else if(!strcmp(campo, "cidadeBebe")) 
            scan_quote_string(cidadeBebe);        //lê o campo cidadeBebe
        else if(!strcmp(campo, "dataNascimento")) 
            scan_quote_string(dataNascimento);    //lê o campo dataNascimento
        else if(!strcmp(campo, "sexoBebe")) 
            scan_quote_string(sexoBebe);          //lê o campo sexoBebe
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
    RegistroCabecalho *cabecalho = ler_cabecalho_bin(bin);
    int encontrados = 0;
    if(cabecalho == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    free(cabecalho);
    cabecalho = NULL;

    RegistroPessoa *wanted = ler_campos();      //recebe um pseudo-registro com valores dos parâmetros de busca
    encontrados = buscar_registros(bin, wanted, &imprimir_registro_aux);
    liberar_registro(&wanted, true);
    
    if(encontrados == 0)
        printf("Registro Inexistente.");
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
        printf("Registro Inexistente.");
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
    if(cabecalho == NULL) {
        printf("Falha no processamento do arquivo.");
        fclose(bin);
        return;
    }

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

    free(cabecalho);                                                    //apaga o registro de cabeçalho
    cabecalho = NULL;

    fclose(bin);                                                        //necessário para a execução da binarioNaTela
    binarioNaTela(bin_pathname);
}


/**
 * Funcionalidade 6 do programa. Lê registros do STDIN e os insere em um arquivo. Fecha o arquivo no fim da execucao.
 * Formato do STDIN: numeroInserções\n
 *                   campo1 campo2 campo3...\n
 *                   campo1 campo2 campo3...\n
 *                   ...
 * 
 * @param bin_pathname nome do arquivo binário; necessário para a chamada da função binarioNaTela.
 * @param bin arquivo binário a ser utilizado.
 */
bool func6(char *bin_pathname, FILE *bin, char *indice_pathname, FILE *indice) 
{
    int n;                                                  // Número de execuções da funçionalidade
    RegistroCabecalho *cabecalho = ler_cabecalho_bin(bin);  // Registro de cabeçalho
    RegistroPessoa *rp = NULL;                              // Registro de dados
    BTCabecalho *bt_cab = bt_ler_cabecalho(indice);         // Registro de cabeçalho do índice

    /* Checa se o arquivo existe e está consistente */
    if(cabecalho == NULL || bt_cab == NULL) {
        if(cabecalho != NULL) {             // Se foi alocado cabeçalho
            free(cabecalho);                // Apaga o cabeçalho
            cabecalho = NULL;
        } else if(bt_cab != NULL) {         // Se foi alocado cabeçalho de índices
            free(bt_cab);                   // Apaga o cabeçalho de índices
            bt_cab = NULL;
        }
        printf("Falha no processamento do arquivo.");   // Mensagem de erro
        fclose(bin);                                    // Fecha o arquivo de dados
        fclose(indice);                                 // Fecha o arquivo de índices
        return false;
    }

    /* Atualiza a consistencia do arquivo */
    atualizar_cabecalho(cabecalho, '0', -1, -1, -1, -1);    // Atualiza o status
    escrever_cabecalho(cabecalho, bin);                     // Escreve no arquivo
    bt_escrever_status('0', indice);

    /* Executa n vezes a funcionalidade */
    scanf("%d", &n);                                        // Lê o número de execuções
    while(n > 0) {
        /* Lê o registro */
        rp = ler_registro_stdin();
        if(rp == NULL) {
            printf("Falha no processamento do arquivo.");           // Mensagem de erro
            atualizar_cabecalho(cabecalho, '1', -1, -1, -1, -1);    // Atualiza o status
            escrever_cabecalho(cabecalho, bin);                     // Escreve o cabeçalho
            free(cabecalho);                                        // Apaga o cabeçalho
            cabecalho = NULL;
            if(bt_cab != NULL) {
                free(bt_cab);                                       // Apaga o cabeçalho do índice
                bt_cab = NULL;
            }
            fclose(bin);                                            // Fecha o arquivo de dados
            fclose(indice);                                         // Fecha o arquivo de índices
            return false;
        }

        /* Insere o registro */
        fseek(bin, ((long) proximo_rrn(cabecalho) * 128) + 128, SEEK_SET);  // Procura a posição a ser inserida
        registro2bin(rp, bin);                                              // Insere o registro

        /* Indexa o registro no índice árvore-B */
        bt_inserir(registro_idNascimento(rp), proximo_rrn(cabecalho), bt_cab, indice);

        /* Atualiza as informações de cabeçalho */
        atualizar_cabecalho(cabecalho, -1, proximo_rrn(cabecalho) + 1, qnt_registros_inseridos(cabecalho) + 1, -1, -1);

        /* Prepara para o próximo loop */
        liberar_registro(&rp, true);    // Desaloca o registro atual
        n--;                            // Diminui o contador de execuções restantes
    }

    atualizar_cabecalho(cabecalho, '1', -1, -1, -1, -1);    // Atualiza o status do cabeçalho do arquivo de dados

    /* Escrevendo cabeçalhos atualizados */
    escrever_cabecalho(cabecalho, bin);                     
    free(cabecalho);                                        

    bt_escrever_cabecalho(bt_cab, indice);
    bt_escrever_status('1', indice);
    free(bt_cab);

    /* Finalizando */
    fclose(bin);  
    fclose(indice);
    return true;                                          
}


/**
 * Funcionalidade 7 do programa. Atualiza os registros especificados no STDIN. Fecha o arquivo no fim da execucao.
 * Formato do STDIN: numeroAtualizações\n
 *                   RRN numCampos nomeCampo valorCampo [nomeCampo valorCampo [...\n
 *                   RRN numCampos nomeCampo valorCampo [nomeCampo valorCampo [...\n
 *                   ...
 * 
 * @param bin_pathname nome do arquivo binário; necessário para a chamada da função binarioNaTela.
 * @param bin arquivo binário a ser utilizado.
 */
void func7(char *bin_pathname, FILE *bin) {
    int n;                                                  // Número de execuções da funçionalidade
    RegistroCabecalho *cabecalho = ler_cabecalho_bin(bin);  // Registro de cabeçalho
    RegistroPessoa *rp = NULL;                              // Registro de dados
    RegistroPessoa *atualizacao = NULL;                     // Registro com os campos a serem atualizados
    int rrn;
    /* Checa se o arquivo existe e está consistente */
    if(cabecalho == NULL) {
        printf("Falha no processamento do arquivo.");   // Mensagem de erro
        fclose(bin);                                    // Fecha o arquivo
        return;
    }

    /* Atualiza a consistencia do arquivo */
    atualizar_cabecalho(cabecalho, '0', -1, -1, -1, -1);    // Atualiza o status
    escrever_cabecalho(cabecalho, bin);                     // Escreve no arquivo

    scanf("%d", &n);    // Lê o número de execuções

    /* Executa n vezes a funcionalidade */
    while(n > 0) {
        /* Decrementa o numero de iteracoes restantes */
        n--;
        /* Leitura da entrada do usuário */
        scanf(" %d", &rrn);              // Lê o RRN do registro a ser modificado
        atualizacao = ler_campos();     // Lê os campos a serem modificados
        
        /* Busca do registro especificado pelo RRN */
        rp = registro_em(rrn, bin);     // Busca o registro
        if(rp == NULL) {
            liberar_registro(&atualizacao, true);   // Apaga o registro de campos modificados da memória
            continue;                               // Passa para a próxima iteração caso o registro não exista
        }
        
        /* Atualiza o registro e insere no arquivo */
        atualizar_registro(rp, atualizacao);    // Atualiza o registro
        atualizar_mantendo_lixo(rp, bin, rrn);  // Atualiza no arquivo mantendo o lixo
        atualizar_cabecalho(cabecalho, -1, -1, -1, -1, qnt_registros_atualizados(cabecalho) + 1);
        
        /* Prepara para a próxima iteração */
        liberar_registro(&rp, true);            // Apaga o registro da memória
        liberar_registro(&atualizacao, true);   // Apaga o registro de campos modificados da memória
    }

    /* Finaliza a funcionalidade */ 
    atualizar_cabecalho(cabecalho, '1', -1, -1, -1, -1);    // Atualiza o status
    escrever_cabecalho(cabecalho, bin);                     // Escreve o cabeçalho
    free(cabecalho);                                        // Apaga o cabeçalho
    cabecalho = NULL;
    fclose(bin);                                            // Fecha o arquivo
    binarioNaTela(bin_pathname);                            // Chama o binarioNaTela()
}


/**
 * Funcionalidade 8 do programa. Cria um arquivo de índice árvore-B a partir de um arquivo de dados já existente.
 * 
 * @param bin arquivo de dados.
 * @param indice_pathname nome do arquivo de índice a ser criado.
 */
void func8(FILE *bin, char *indice_pathname) {
    if(bt_criar(bin, indice_pathname))
        binarioNaTela(indice_pathname);
    else 
        printf("Falha no processamento do arquivo.");  
}


/**
 * Funcionalidade 9 do programa. Recupera um registro com base em uma chave de busca e imprime.
 * Formato da entrada no STDIN: idNascimento valor
 * 
 * @param bin arquivo contendo os registros de dados.
 * @param indice arquivo contendo a árvore-B.
 */
void func9(FILE *bin, FILE *indice) {
    char tipoChave[20];                                 // Guarda o tipo da chave recebido pela entrada
    int valor;                                          // Guarda a chave recebida pela entrada
    int numAcessos, rrn;                                // Guarda os resultados da busca pela chave
    RegistroCabecalho *cab = ler_cabecalho_bin(bin);    // Guarda o cabeçalho do arquivo de dados
    RegistroPessoa *rp = NULL;                          // Guarda o registro de dados encontrado da busca

    /* Testa a leitura do cabeçalho do arquivo de dados para checagem de consistência */
    if(cab == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    /* Apaga a struct de cabeçalho */
    free(cab);
    cab = NULL;

    /* Lê a entrada do usuário */
    scanf(" %19s", tipoChave);                          // Lê o tipo de chave
    scanf("%d", &valor);                                // Lê o valor da chave

    /* Realiza a busca */
    numAcessos = bt_busca(&rrn, valor, indice);         // Busca e retorna o número de acessos a páginas de disco
    if(numAcessos == NIL) {
        /* Mensagem de falha no processamento */
        printf("Falha no processamento do arquivo.");
        return;
    }

    /* Procura o registro no arquivo de dados */
    rp = registro_em(rrn, bin);
    if(rp != NULL) {
        /* Lê o registro encontrado e imprime */
        imprimir_registro_formatado(rp);                // Imprime o registro
        liberar_registro(&rp, true);                    // Apaga a struct de registro
        /* Imprime o número de páginas da árvore-B acessadas */
        printf("Quantidade de paginas da arvore-B acessadas: %d\n", numAcessos);
    } else {
        /* Mensagem de registro não encontrado */
        printf("Registro inexistente.");
    }
}