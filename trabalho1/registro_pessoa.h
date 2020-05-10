/**
 * Contém a definição das estruturas dos registros tais como as funções que possibilitam criá-los e operá-los.
 */

#ifndef REGISTRO_PESSOA_H
    #define REGISTRO_PESSOA_H
    #include <stdio.h>
    #include <stdbool.h>

    typedef struct Dados RegistroPessoa;
    typedef struct Cabecalho RegistroCabecalho;

    RegistroPessoa* ler_registro(char *line);
    int existe_registros(RegistroCabecalho *cabecalho);
    void liberar_registro(RegistroPessoa **rp, bool liberar_variaveis);

    RegistroCabecalho* ler_cabecalho_bin(FILE *bin);
    RegistroCabecalho* criar_cabecalho(void);
    void escrever_cabecalho(RegistroCabecalho *c, FILE *bin);
    void atualizar_cabecalho(RegistroCabecalho *c, char status, int RRNproxRegistro, int numeroRegistrosInseridos, int numeroRegistrosAtualizados);
    void liberar_cabecalho(RegistroCabecalho **c);

    void registro2bin(RegistroPessoa *rp, FILE *bin);
    RegistroPessoa *ler_registro_bin(FILE *bin);
    void imprimir_registro(RegistroPessoa *rp);
#endif