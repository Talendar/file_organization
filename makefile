run:
	./programaTrab

all: clean main.o funcionalidades.o csv_bin_manager.o registro_pessoa.o arvore_b.o
	gcc arvore_b.o registro_pessoa.o csv_bin_manager.o funcionalidades.o main.o -o programaTrab

main.o: main.c
	gcc -c main.c

funcionalidades.o: funcionalidades.c funcionalidades.h
	gcc -c funcionalidades.c

csv_bin_manager.o: csv_bin_manager.c csv_bin_manager.h
	gcc -c csv_bin_manager.c

registro_pessoa.o: registro_pessoa.c registro_pessoa.h
	gcc -c registro_pessoa.c
	
arvore_b.o: arvore_b.c arvore_b.h
	gcc -c arvore_b.c

clean:
	rm -rf *.o programaTrab
