# Creation of the entire program
CFLAGS = -Wall -W -pedantic

KMP_parallel : KMP_parallel.o KMP.o
	mpicc $(CFLAGS) -o KMP_parallel KMP_parallel.o KMP.o

KMP_parallel_o : KMP_parallel_o.o KMP.o
	mpicc $(CFLAGS) -o KMP_parallel_o KMP_parallel_o.o KMP.o

# Compilazione dell'algoritmo parallelo e del main
KMP_parallel.o: KMP_parallel.c
	mpicc $(CFLAGS) -c KMP_parallel.c

KMP_parallel_o.o: KMP_parallel_o.c
	mpicc $(CFLAGS) -c KMP_parallel_o.c

# Compilazione dell'algoritmo classico
KMP.o: KMP.c
	mpicc $(CFLAGS) -c KMP.c

clean:
	rm -f *.o
