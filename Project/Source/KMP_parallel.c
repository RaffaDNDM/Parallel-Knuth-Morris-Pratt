/**
  @file KMP_parallel.c
  @brief Classe con il main e la computazione di KMP in parallelo
  @author Di Nardo Di Maio Raffaele 1204879
  @author Fabris Cristina 1205722
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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
void our_min(const int *invector, int *outvalue, const int *size, const MPI_Datatype *dtype);


int main (int argc, char **argv)
{
    //Inizializzazione di MPI
    MPI_Init(&argc, &argv);

    //rank di ogni processore e numero di processori
    int rank, size;

    //inizializzazione dei valori precedenti
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    //inizializzazione della dimensione del testo
    int size_text=SIZE;

    //stringhe che contengono rispettivamente l'intero testo, l'insieme
    //di tutte stringhe usate poi nel secondo ciclo e il pattern
    char *text,*text2, *pattern= argv[2];

    //grandezza complessiva del testo formato da tutte le stringhe analizzate
    //dai processori nel secondo ciclo dell'algoritmo KMP parallelo (= strlen(text2))
    int size_cycle2;

    //lunghezza del pattern
    int m = strlen(pattern);

    //failure vector che verrà calcolato dal processore 0
    //una volta per tutte e poi condivisa con gli altri processori
    int *fail;
    fail=malloc(sizeof(int)*m);

    //array contenente gli indici (all'interno del testo originale) del primo
    //carattere di tutte le stringhe passate ai processori nel secondo ciclo
    int *indices = malloc(sizeof(int)*size);

    //interi utilizzati per definire la grandezza della stringa
    //letta da ciascun processore nel primo ciclo dell'algoritmo
    int rank_size=0, last_extra=0;

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
        int fp = open(argv[1], O_RDONLY);
        int i=0;
        int check=1;

        while(check!=0)
        {
            if(i==size_text)
            {
                size_text=size_text<<1;
                text=realloc(text, sizeof(char)*size_text);
            }

            check=read(fp, &(text[i]), 1);
            i=i+1;
        }

        size_text=i-1;//size_text corrisponde al numero di caratteri del testo in input
        text=realloc(text, sizeof(char)*(size_text+1));
        text[size_text]=0;

        //Chiusura del file aperto in lettura
        close(fp);

        /*
        if(size_text<m)
        {
            printf("Il pattern non è presente all'interno del testo\n");
            return 0;
        }
        */

        //inizializzazione del fail vector e sua computazione
        i=0;
        for(;i<m;i++)
        {
            fail[i]=0;
        }
        computeFailKMP(pattern, m, fail);

        //lunghezza della stringa divisa tra i vari processori nel primo ciclo (testo - ultimi (m-1) caratteri)
        int true_size=size_text-m+1;
        //numero di caratteri che ogni processo andrà ad analizzare (ad esclusione dell'ultimo)
        rank_size=true_size/size;
        //numero di caratteri da aggiungere a quelli che analizza l'ultimo processo
        last_extra=true_size%size;
        //indice del primo carattere da leggere (inizializzata ad un valore non valido)
        int begin_r=-1;
        //indice della posizione in cui scirvere il primo carattere (inizializzata ad un valore non valido)
        int begin_w=-1;
        //lunghezza di una stringa che verrà passata a un processore nel secondo ciclo
        int length_string2=(m-1)<<1;
        //numero di caratteri che ogni processo andrà ad analizzare al secondo ciclo
        size_cycle2=size*(length_string2)+1;

        text2=malloc(sizeof(char)*size_cycle2);

        for (i=0; i<size-1; i++)
        {
            int j=0;
            begin_r=((i+1)*rank_size)-m+1;

            //memorizzazione degli indici del primo carattere passato successivamente ai processori per il secondo ciclo
            indices[i]=begin_r;

            //acquisizione del testo necessario per il secondo ciclo
            begin_w=i*length_string2;
            for(;j<length_string2;j++)
            {
                text2[begin_w+j]=text[begin_r+j];
            }
        }

        //acquisizione degli ultimi caratteri del testo necessari per il secondo ciclo
        begin_r=size_text-length_string2;
		    indices[size-1]=begin_r;

		    begin_w=i*length_string2;
		    int j=0;

        for (; j<length_string2; j++)
        {
            text2[begin_w+j]=text[begin_r+j];
        }
        text2[size_cycle2-1]=0;

        //dati MPI usata per la comunicazione punto a punto successiva
        MPI_Request req;
        //invio dei caratteri da aggiungere a quelli da analizzare all'ultimo processo
        MPI_Isend(&last_extra, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD, &req);
    }

    //invio della dimensione del buffer da allocare in ogni processo per ricevere il testo da analizzare
    MPI_Bcast(&rank_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //dichiarazione della grandezza della stringa che verrà passata a ciascun processore nel primo ciclo
    int rcv_size=rank_size;
    MPI_Status stat;
    if (rank==size-1)
    {
        //ricezione dell'ultimo processo del numero di caratteri in più da leggere rispetto agli altri processi
        MPI_Recv(&last_extra, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &stat);
        rcv_size=rcv_size+last_extra;
    }

    //stringa analizzata da ciascun processo al primo ciclo
    char *rcv_buff=malloc(sizeof(char)*(rcv_size+1));

    //stringa analizzata da ciascun processo al secondo ciclo
    char *rcv_buff2=malloc(sizeof(char)*((m<<1)-1));
    MPI_Barrier(MPI_COMM_WORLD);

    //passaggio di rank_size caratteri a ciascun processore (verranno analizzati al primo ciclo)
    MPI_Scatter(text, rank_size, MPI_CHAR, rcv_buff, rank_size, MPI_CHAR, 0, MPI_COMM_WORLD);
	  MPI_Barrier(MPI_COMM_WORLD);

    //passaggio dei restanti caratteri necessari all'ultimo processore (last_extra)
    //per definire la stringa che analizzerà nel primo ciclo
    if(rank==0)
    {
        //dati MPI usata per la comunicazione punto a punto successiva
        MPI_Request req;
        MPI_Isend(&text[size*rank_size], last_extra, MPI_CHAR, size-1, 1, MPI_COMM_WORLD, &req);
    }

    if(rank==size-1)//METTERE UN IF ELSE?
    {
        MPI_Recv(&rcv_buff[rank_size], last_extra, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);
    }
	  rcv_buff[rcv_size] = 0;

    //comunicazione dell'array fail a tutti i processi
    MPI_Bcast(fail, m, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //comunicazione a tutti i processi delle stringhe da analizzare nel secondo ciclo
    MPI_Scatter(text2, (m-1)<<1, MPI_CHAR, rcv_buff2,(m-1)<<1, MPI_CHAR, 0, MPI_COMM_WORLD);

    //indice del primo carattere della stringa, passata al processore
    //nel secondo ciclo dell'algoritmo, rispetto al testo originale
    int index=-1;

	  //passaggio dell'indice trovare il primo carattere del secondo ciclo nel testo complessivo
    //(necessario per il calcolo dell'indice assoluto di inizio dell'occorenza del pattern)
    MPI_Scatter(indices, 1, MPI_INT, &index, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //calcolo dell'indice di inizio della prima occorrenza del pattern all'interno del testo
    //utilizzando l'algoritmo KMP
    int partial_result= findKMP(rcv_buff, pattern, m, fail);
    if(partial_result!=-1)
    {
        //--------------Ciclo 1------------------------
        partial_result=partial_result+(rank_size*rank);//calcolo dell'indice assoluto
    }
	  else
    {
        //--------------Ciclo 2------------------------
	      partial_result= findKMP(rcv_buff2, pattern, m, fail);
        if(partial_result!=-1)
        {
    		    partial_result=partial_result+index;//calcolo dell'indice assoluto
        }
	  }

    //creazione della nostra funzione per il calcolo dell'indice minimo tra le occorenze trovate
    MPI_Op operation;
    MPI_Op_create((MPI_User_function *) our_min, 1, &operation);
    MPI_Barrier(MPI_COMM_WORLD);

    //indice della prima occorrenza del pattern nel testo (-1 se non è presente)
    int result = -1;

    //comunicazione dell'indice minimo tra quelli valutati da ciascun processore
    MPI_Reduce(&partial_result, &result, 1, MPI_INT, operation, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    //stampa dell'indice trovato
    if(rank==0)
    {
        printf(LINE);
        if(result!=-1)
        {
            printf("  Il pattern è presente nel testo all'indice: %12d\n", result);
        }
        else
        {
            printf("Il pattern non è presente all'interno del testo\n");
        }
        printf(LINE);
    }

    //Deallocazione della memoria
    MPI_Op_free(&operation);

    free(indices);
    free(rcv_buff);
    free(rcv_buff2);
    free(fail);

    if (rank==0)
    {
        free(text2);
        free(text);

		    //Chiusura dell'applicazione
		    printf("Premere un tasto qualsiasi per uscire\n");
		    getchar();

		    //Chiusura dell'applicazione
		    printf("Premere un tasto qualsiasi per uscire\n");
		    getchar();

    }

    //Termine di MPI
    MPI_Finalize();
    return 0;
}

void our_min(const int *invector, int *outvalue, const int *size, const MPI_Datatype *dtype)
{
    int i=0;
    while(i<(*size))
    {
        int k=0;

        if(invector[i]!=-1)
        {
            // se primo indice valido che trovo inizializzo il valore di output
            if(k==0)
            {
                *outvalue=invector[i];
            }
            //altrimenti cerco il minimo devo scandirli perchè l'ordine in invector
            //dipende dall'arrivo dei messaggi nel buffer
            //potrebbe non rispettare l'ordine crescente del rank dei processi
            if(invector[i]<*outvalue)
            {
                *outvalue=invector[i];

            }
            k=k+1;
        }
        i=i+1;
    }
}
