/**
  @file KMP_parallel.h
  @brief Definizione di macro utili per l'elaborazione dell'input
  @brief e dell'output in Parallel KMP
  @author Di Nardo Di Maio Raffaele 1204879
  @author Fabris Cristina 1205722
*/

#ifndef PARALLEL_KMP

#define PARALLEL_KMP
#define SIZE 50
#define MAX_BUFF_SIZE 50*1024*1024
#define LINE "+----------------------------------------------------------+\n"
#define OUT_SIZE 58
#define TITLE "\n \
 _____                _ _      _     _  ____  __ _____  \n \
|  __ \\              | | |    | |   | |/ /  \\/  |  __ \\ \n \
| |__) |_ _ _ __ __ _| | | ___| |   | ' /| \\  / | |__) |\n \
|  ___/ _` | '__/ _` | | |/ _ \\ |   |  < | |\\/| |  ___/ \n \
| |  | (_| | | | (_| | | |  __/ |   | . \\| |  | | |     \n \
|_|   \\__,_|_|  \\__,_|_|_|\\___|_|   |_|\\_\\_|  |_|_|     \n\n\n"

int KMP_parallel_function(int* input_text,int n_iteration, MPI_Comm *comm, char *pat, int *finish);


#endif
