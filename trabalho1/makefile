run: programaTrab
	./programaTrab

all: clean main.o csv_bin_manager.o registro_pessoa.o
	gcc registro_pessoa.o csv_bin_manager.o main.o -o programaTrab

main.o: main.c
	gcc -c main.c

csv_bin_manager.o: csv_bin_manager.c csv_bin_manager.h
	gcc -c csv_bin_manager.c

registro_pessoa.o: registro_pessoa.c registro_pessoa.h
	gcc -c registro_pessoa.c

clean:
	rm -rf *.o programaTrab
