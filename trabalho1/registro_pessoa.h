#ifndef REGISTRO_PESSOA_H
    #define REGISTRO_PESSOA_H
    #include <stdio.h>
    typedef struct Dados RegistroPessoa;
    typedef struct Cabecalho RegistroCabecalho;

    RegistroCabecalho *ler_cabecalho_bin(FILE *bin);
    RegistroPessoa* ler_registro(char *line);
    int existeRegistros(RegistroCabecalho *cabecalho);
    void liberar_registro(RegistroPessoa **rp);

    RegistroCabecalho* criar_cabecalho();
    void escrever_cabecalho(RegistroCabecalho *c, FILE *bin);
    void atualizar_cabecalho(RegistroCabecalho *c, char status, int RRNproxRegistro, int numeroRegistrosInseridos, int numeroRegistrosAtualizados);

    void registro2bin(RegistroPessoa *rp, FILE *bin);
    void imprimir_registro(RegistroPessoa *rp);

    RegistroPessoa *ler_registro_bin(FILE *bin);
    void imprimir_registro_formatado(RegistroPessoa *rp);
#endif