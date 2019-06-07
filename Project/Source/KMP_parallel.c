/**
  @file KMP_parallel.cù
  @brief Classe con ilmain e la computazione di KMP in parallelo
  @author Di Nardo Di Maio Raffaele 1204879
  @author Fabris Cristina 1205722
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "KMP.h"
#include "KMP_parallel.h"

/**
  @details Operatore MPI che valuta il minimo tra gli indici validi delle occorrenze del pattern
  @param invector buffer di valori in input
  @param outvalue puntatore all'indice valido minimo tra quelli in invector
  @param size grandezza del buffer di input (grandezza del comunicatore poi nel Reduce)
  @param dtype tipo di dato MPI generico
*/
void our_min(int *invector, int *outvalue, int *size, MPI_Datatype *dtype);


int main (int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    //rank di ogni processore e numero di processori
    int rank, size;

    //inizializzazione dei valori precedenti
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    //dati MPI poi usata per la comunicazione punto a punto successiva
    MPI_Request req;
    MPI_Status stat;

    //inizializzazione del buffer contenente l'intero testo
    int *size_buff;
    int size_text=SIZE;

    //grandezza delle stringhe analizzate da ciscun processore
    //nel secondo ciclo dell'algoritmo KMP parallelo
    int size_cycle2;

    //stringhe che contengono rispettivamente l'intero testo, l'insieme
    //delle stringhe usate poi nel secondo ciclo e il pattern
    char *text,*text2, *pattern= argv[2];
    //lunghezza del pattern
    int m = strlen(pattern);

    //failure function che verrà calcolata dal processore 0
    //per tutti gli altri e poi condivisa con loro
    int *fail;
    fail=malloc(sizeof(int)*m);


	  //indice del primo carattere della stringa passata
    //al processore nel secondo ciclo dell'algoritmo
    int index=-1;
    //array di tutti gli indici dei primi caratteri di
    //tutte le stringhe passate ai processori nel secondo ciclo
    int *indices = malloc(sizeof(int)*size);

    //variabili utilizzate per definire poi la grandezza del buffer finale
    int rank_size=0, last_extra=0, rcv_size=0;

    //*************************** RANK 0 ********************************
    if (rank==0)
    {
        printf(TITLE);

        //verifica della correttezza dei parametri di input
        if(argc<2)
        {
            printf("Mancanti parametri di input (nome_programma filepath pattern)");
            exit(1);
        }

        //acquisizione del testo in cui cercare il pattern
        text = malloc(sizeof(char)*size_text);
        FILE *fp= fopen(argv[1], "r");
        int i=0;

        while(!feof(fp))
        {
            if(i==size_text)
            {
                size_text=size_text*2;
                text=realloc(text, sizeof(char)*size_text);
            }

            text[i]=fgetc(fp);
            i++;
        }

        size_text=i-1;//size_text corrisponde al numero di caratteri del testo in input
        text=realloc(text, sizeof(char)*(size_text+1));
        text[size_text]=0;

        //allocazione dello spazio per l'array fail e sua computazione, m=lunghezza del pattern
        i=0;
        for(;i<m;i++)
        {
            fail[i]=0;
        }
        computeFailKMP(pattern, m, fail);

        rank_size=(size_text-m+1)/size;//numero di caratteri che ogni processo andrà ad analizzare (ad esclusione dell'ultimo)
        last_extra=(size_text-m+1)%size;//numero di caratteri da aggiungere a quelli che analizza l'ultimo processo

        int begin_r=-1;//variabile che contiene l'indice del primo carattere da leggere (inizializzata ad un valore non valido)
        int begin_w=-1;//variabile che contiene l'indice della posizione in cui scirvere il primo carattere (inizializzata ad un valore non valido)
        size_cycle2=size*((2*(m-1)))+1;//numero di caratteri che ogni processo andrà ad analizzare al secondo ciclo
        text2=malloc(sizeof(char)*size_cycle2);

        for (i=0; i<size-1; i++)
        {
            int j=0;
            begin_r=((i+1)*rank_size)-m+1;

			indices[i]=begin_r;//memorizzazione degli indici del primo carattere passato successivamente ai processori per il secondo ciclo

            //acquisizione del testo necessario per il secondo ciclo
			begin_w=i*2*(m-1);
            for(;j<2*(m-1);j++)
            {
                text2[begin_w+j]=text[begin_r+j];
            }
        }

        //acquisizione degli ultimi caratteri del testo necessari per il secondo ciclo
        begin_r=size_text-(2*(m-1));
		indices[size-1]=begin_r;

		begin_w=i*(2*(m-1));
		int j=0;

        for (; j<2*(m-1); j++)
        {
            text2[begin_w+j]=text[begin_r+j];
        }
        text2[size_cycle2-1]=0;

        //invio dei caratteri da aggiungere a quelli da analizzare all'ultimo processo
        MPI_Isend(&last_extra, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD, &req);
    }

    //invio della dimensione del buffer da allocare in ogni processo per ricevere il testo da analizzare
    MPI_Bcast(&rank_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    rcv_size=rank_size;
    if (rank==size-1)
    {
        //ricezione da parte dell'ultimo processo del numero di caratteri da aggiungere rispetto agli altri processi
        MPI_Recv(&last_extra, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &stat);
        rcv_size=rcv_size+last_extra;
    }

	char *rcv_buff=malloc(sizeof(char)*(rcv_size+1));
    char *rcv_buff2=malloc(sizeof(char)*((2*m)-1));
    MPI_Barrier(MPI_COMM_WORLD);

    //passaggio di rank_size caratteri a ciascun processore (verranno analizzati al primo ciclo)
    MPI_Scatter(text, rank_size, MPI_CHAR, rcv_buff, rank_size, MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

    //passaggio del resto di caratteri all'ultimo processore (last_extra)
    //(nel caso in cui il numero di caratteri del testo non sia multiplo del numero dei processori)
    if(rank==0)
    {
        MPI_Isend(&text[size*rank_size], last_extra, MPI_CHAR, size-1, 1, MPI_COMM_WORLD, &req);
    }

    if(rank==size-1)
    {
        MPI_Recv(&rcv_buff[rank_size], last_extra, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);
    }
	rcv_buff[rcv_size] = 0;

    //passaggio dell'array fail a tutti i processi
    MPI_Bcast(fail, m, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //passaggio a tutti i processi dei caratteri da analizzare al secondo ciclo
    MPI_Scatter(text2, 2*(m-1), MPI_CHAR, rcv_buff2, 2*(m-1), MPI_CHAR, 0, MPI_COMM_WORLD);


	//passaggio dell'indice trovare il primo carattere del secondo ciclo nel testo complessivo
    //(necessario per il calcolo dell'indice assoluto di inizio dell'occorenza del pattern)
	MPI_Scatter(indices, 1, MPI_INT, &index, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //calcolo dell'indice di inizio della prima occorrenza del pattern all'interno del testo
    //utilizzando l'algoritmo KMP
    int partial_result= findKMP(rcv_buff, pattern, m, fail);
    if(partial_result!=-1)
    {
        //--------------Ciclo 1-----------------------
        partial_result=partial_result+(rank_size*rank);//calcolo dell'indice assoluto
    }
	else
    {
        //------------Ciclo 2-------------------------
	    partial_result= findKMP(rcv_buff2, pattern, m, fail);
        if(partial_result!=-1)
        {
    		partial_result=partial_result+index;//calcolo dell'indice assoluto
        }
	}


	int result = -1;

    //creazione della nostra funzione per il calcolo dell'indice minimo tra le occorenze trovate
    MPI_Op operation;
    MPI_Op_create((MPI_User_function *) our_min, 1, &operation);
    MPI_Barrier(MPI_COMM_WORLD);
    //calcolo dell'indice minimo
    MPI_Reduce(&partial_result, &result, 1, MPI_INT, operation, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //stampa dell'indice trovato (se trovato)
    if(rank==0)
    {
        printf(LINE);
        if(result==-1)
        {
            printf("Il pattern non è presente all'interno del testo\n");
        }
        else
        {
            printf("  Il pattern è presente nel testo all'indice: %12d\n", result);
        }
        printf(LINE);
    }

    MPI_Op_free(&operation);

    if (rank==0)
    {
        free(text);
        free(fail);
    }
    getchar();
    MPI_Finalize();
    return 0;
}

void our_min(int *invector, int *outvalue, int *size, MPI_Datatype *dtype)
{
    int flag=1;
    int i=0;
    while(i<(*size))
    {
        int k=0;

        if(invector[i]!=-1)
        {
            // se primo indice valido che trovo inizializzo il valore di output
            // (indice minore poichè valori passati in input già ordinati dal più
            //  piccolo al più grande per costruzione)
            if(k==0)
            {
                *outvalue=invector[i];
            }
            //SIAMO SICURI CHE QUESTO IF SERVA?
            if(invector[i]<*outvalue)
            {
                *outvalue=invector[i];

            }
            k=k+1;
        }
        i=i+1;
    }
}
