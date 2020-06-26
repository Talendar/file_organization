/**
 * Este módulo implementa o necessário para lidar com registros de dados em um arquivo.
 */


#ifndef REGISTRO_PESSOA_H
    #define REGISTRO_PESSOA_H
    #include <stdio.h>
    #include <stdbool.h>

    /* Structs */
    typedef struct Dados RegistroPessoa;
    typedef struct Cabecalho RegistroCabecalho;

    /* Leitura e criação de registros de dados */
    RegistroPessoa* ler_registro_csv(char *line);
    RegistroPessoa* criar_registro(char *cidadeMae, char *cidadeBebe, int idNascimento, int idadeMae, char dataNascimento[11], char sexoBebe[2], char estadoMae[3], char estadoBebe[3]);
    RegistroPessoa* ler_registro_bin(FILE *bin);
    RegistroPessoa* registro_em(int rrn, FILE *bin);
    RegistroPessoa* ler_registro_stdin(void);

    /* Remoção de registros de dados */
    void remover_registro(FILE *bin);
    void remover_registro_aux(FILE *bin, RegistroPessoa *rp);

    /* Consulta a registro de dados */
    int registro_idNascimento(RegistroPessoa *rp);

    /* Funcionalidades adicionais de registros de dados */
    bool registros_similares(RegistroPessoa *modelo, RegistroPessoa *candidato);
    int buscar_registros(FILE *bin, RegistroPessoa *modelo, void (*func)(FILE *bin, RegistroPessoa *rp));
    bool verificar_removido(FILE *bin);
    void pular_registro(FILE *bin);
    void atualizar_registro(RegistroPessoa *original, RegistroPessoa *alteracoes);
    void atualizar_mantendo_lixo(RegistroPessoa *rp, FILE *bin, int rrn);

    /* Desalocação de memória */
    void liberar_registro(RegistroPessoa **rp, bool liberar_campos_variaveis);

    /* Leitura, criação e manipulação de registros de cabeçalho */
    RegistroCabecalho* ler_cabecalho_bin(FILE *bin);
    RegistroCabecalho* criar_cabecalho(void);
    void escrever_cabecalho(RegistroCabecalho *c, FILE *bin);
    void atualizar_cabecalho(RegistroCabecalho *c, char status, int RRNproxRegistro, int numeroRegistrosInseridos, int numeroRegistrosRemovidos, int numeroRegistrosAtualizados);

    /* Consultas a registros de cabeçalho */
    int proximo_rrn(RegistroCabecalho *cabecalho);
    int existe_registros(RegistroCabecalho *cabecalho);
    int qnt_registros_inseridos(RegistroCabecalho *c);
    int qnt_registros_removidos(RegistroCabecalho *c);
    int qnt_registros_atualizados(RegistroCabecalho *c);

    /* Impressão no STDOUT */
    void imprimir_registro_teste(RegistroPessoa *rp);
    void imprimir_registro_formatado(RegistroPessoa *rp);
    void imprimir_registro_aux(FILE *bin, RegistroPessoa *rp);
    void imprimir_cabecalho_teste(RegistroCabecalho *c);

    /* Funcionalidades adicionais */
    void registro2bin(RegistroPessoa *rp, FILE *bin);
    bool fim_do_arquivo(FILE *bin);    
#endif