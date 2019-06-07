#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <limits.h>
#ifndef PARALLEL_KMP
#define PARALLEL_KMP
#define SIZE 60
#define LINE "+----------------------------------------------------------+\n"
#define OUT_SIZE 58
#define TITLE "\n \
 _____                _ _      _     _  ____  __ _____  \n \
|  __ \\              | | |    | |   | |/ /  \\/  |  __ \\ \n \
| |__) |_ _ _ __ __ _| | | ___| |   | ' /| \\  / | |__) |\n \
|  ___/ _` | '__/ _` | | |/ _ \\ |   |  < | |\\/| |  ___/ \n \
| |  | (_| | | | (_| | | |  __/ |   | . \\| |  | | |     \n \
|_|   \\__,_|_|  \\__,_|_|_|\\___|_|   |_|\\_\\_|  |_|_|     \n\n\n"

int findKMP(const char *text, const char *pattern, int m, const int *fail);
void computeFailKMP(char * pattern, int m, int *fail);
void our_min(int *invector, int *outvalue, int *size, MPI_Datatype *dtype);

int main (int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Request req;
    MPI_Status stat;
    int *size_buff;
    int size_text=SIZE;
    int size_cycle2;
    int *fail;
    char *text,*text2, *pattern= argv[2];
    int m = strlen(pattern);
    int rank_size=0, last_extra=0, rcv_size=0;
    fail=malloc(sizeof(int)*m);


	//
	int index=-1;
	int *indices = malloc(sizeof(int)*size);


	/**
    *************************** RANK 0 ********************************
    */
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
    /**
    ***********************FINE RANK 0 ********************************
    */

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

/**
* nostra funzione per il calcolo del minimo indice tra le occorrenze del pattern trovate
* invector = vettore con valori di input
* outvalue =indice minimo tra quelli in input
* size = numero processori
* dtype=??
*/
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

/**
 * implementazione dell'algortimo KMP
 * text = testo in cui cercare il pattern
 * pattern = stringa da cercare
 * m = lunghezza del pattern
 * fail = array precalcolato fail
*/
int findKMP(const char *text, const char *pattern, int m, const int *fail)
{
  int n = strlen(text);//lunghezza del testo
  int j=0; //indice che scandisce il testo
  int k=0; //indice che scandisce il pattern

  while (j<n)
  {
    if(text[j]==pattern[k])
    {
      if(k==m-1)// se ho raggiunto la fine del pattern restituisco l'indice di inizio dell'occorrenza
      {
        return j-m+1;
      }

      j=j+1;
      k=k+1;
    }
    else if (k>0) //se non ho corrispondenza ma non sono al primo elemento del pattern
    {
      k=fail[k-1];
    }
    else //se sono all'inizio del pattern e non ho corrispondenza
      {
        j++;
      }
  }
  return -1;//non ha trovato occorrenze
}

/**
 * funzione che computa l'array fail
 * pattern = stringa da cercare nel testo
 * m = lunghezza del pattern
 * fail = array fail da inizializzare
*/
void computeFailKMP(char * pattern, int m, int *fail)
{
   int j=1;//indice che scandisce fail
   int k=0;//indice che scandisce il pattern

   while (j<m)
   {
     if (pattern[j]==pattern[k]) //se presente più volte lo stesso carattere all'interno del pattern
     {
       fail[j]=k+1;
       j=j+1;
       k=k+1;
     }
     else if (k>0) //se non sono all'inizio della scansione del pattern e non è un carattere già presente nel pattern
     {
       k=fail[k-1];
     }
     else // se sono all'inizio della scansione del pattern
     {
       j=j+1;
     }
   }
}

#endif
