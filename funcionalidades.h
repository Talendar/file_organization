/**
 * Este módulo tem por objetivo encapsular a execução das funcionalidades do programa.
 */


#ifndef FUNCIONALIDADES_H
    #define FUNCIONALIDADES_H
    #include <stdio.h>
    #include <stdbool.h>

    void func1(char *csv_pathname, char *bin_pathname);
    void func2(FILE *bin);
    void func3(FILE *bin);
    void func4(FILE *bin);
    void func5(char *bin_pathname, FILE *bin);
    bool func6(char *bin_pathname, FILE *bin, char *indice_pathname, FILE *indice);
    void func7(char *bin_pathname, FILE *bin);
    void func8(FILE *bin, char *indice_pathname);
    void func9(FILE *bin, FILE *indice);
#endif