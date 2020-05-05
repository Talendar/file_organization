#ifndef REGISTRO_PESSOA_H
    #define REGISTRO_PESSOA_H
    #include <stdio.h>
    typedef struct RegistroPessoa RegistroPessoa;

    RegistroPessoa* ler_registro(char *line);
    void liberar_registro(RegistroPessoa **rp);

    void registro2bin(RegistroPessoa *rp, FILE *bin);
    void imprimir_registro(RegistroPessoa *rp);
#endif