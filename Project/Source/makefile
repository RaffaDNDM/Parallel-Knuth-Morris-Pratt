# Creation of the entire program
KMP_parallel : KMP_parallel.o KMP.o
	mpicc -o KMP_parallel KMP_parallel.o KMP.o

# Compilazione dell'algoritmo parallelo e del main
KMP_parallel.o: KMP_parallel.c
	mpicc -c KMP_parallel.c

#Compilazione dell'algoritmo classico
KMP.o: KMP.c
	mpicc -c KMP.c
