#ifndef REGISTRO_PESSOA_H
    #define REGISTRO_PESSOA_H
    #include <stdio.h>
    typedef struct Dados RegistroPessoa;
    typedef struct Cabecalho RegistroCabecalho;

    RegistroCabecalho *ler_cabecalho(FILE *bin);
    RegistroPessoa* ler_registro(char *line);
    int existeRegistros(RegistroCabecalho *cabecalho);
    void liberar_registro(RegistroPessoa **rp);

    void registro2bin(RegistroPessoa *rp, FILE *bin);
    void imprimir_registro(RegistroPessoa *rp);

    RegistroPessoa *ler_registro_bin(FILE *bin);
    int imprimir_registro_formatado(RegistroPessoa *rp);
#endif