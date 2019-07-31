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
#include <libhpc.h>
#include "KMP.h"
#include "KMP_parallel.h"

int main (int argc, char **argv)
{
    hpmInit( 0, argv[3]);
    if (hpm_error_count) exit(4);

    hpmStartx( 1, HPM_ONLY_EXCLUSIVE, "Tempo d'esecuzione");
    if (hpm_error_count) exit(4);


    /*Inizializzazione di MPI*/
    MPI_Init(&argc, &argv);

    /*rank di ogni processore e numero di processori*/
    int rank, size;

    /*inizializzazione dei valori precedenti*/
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    /*inizializzazione della dimensione del testo*/
    int size_text=SIZE;

    /*
    stringhe che contengono rispettivamente l'intero testo, l'insieme
    di tutte stringhe usate poi nel secondo ciclo e il pattern
    */
    char  *pattern= argv[2];

    /*
    grandezza complessiva del testo formato da tutte le stringhe analizzate
    dai processori nel secondo ciclo dell'algoritmo KMP parallelo (= strlen(text2))
    */
    int size_cycle2;

    /*lunghezza del pattern*/
    int m = strlen(pattern);

    /*
    Allocazione e inizializzazione del failure vector, che verrà calcolato dal processore 0
    una volta per tutte e poi condiviso con gli altri processori
    */
    int *fail;
    fail=calloc(m, sizeof(int));

    /*
    array contenente gli indici (all'interno del testo originale) del primo
    carattere di tutte le stringhe passate ai processori nel secondo ciclo
    */
    int *indices = malloc(sizeof(int)*size);

    /*
    interi utilizzati per definire la grandezza della stringa
    letta da ciascun processore nel primo ciclo dell'algoritmo
    */
    int rank_size=0, last_extra=0;

    /*lunghezza di una stringa che verrà passata a un processore nel secondo ciclo*/
    int length_string2=(m-1)<<1;

    int offset=0;

    int fp;

    /*indice della prima occorrenza del pattern nel testo (-1 se non è presente)*/
    int result = -1;

    /*
    array di caratteri necessari per memorizzare rispettivamente
    il testo ricevuto in input
    l'insieme delle stringhe da elaborare nel secondo ciclo dell'algoritmo parallelo
    la porzione di testo che verrà elaborata da ogni processore nel primo ciclo
    la porzione di testo che verrà elaborata da ogni processore nel secondo ciclo
    */
    char *text, *text2, *rcv_buff, *rcv_buff2;

    /*valore dell'indice dell'occorenza trovata da ogni processore con l'algoritmo KMP*/
    int partial_result;


    /**************************** RANK 0 *********************************/
    if (rank==0)
    {
        printf(TITLE);

        /*verifica della correttezza dei parametri di input*/
        if(argc<3)
        {
            printf("Mancanti parametri di input (nome_programma filepath pattern)");
            exit(1);
        }



        fp = open(argv[1], O_RDONLY);

        /*computazione dell'array fail*/
        computeFailKMP(pattern, m, fail);
    }
    if (size>1)
    {
            /*comunicazione dell'array fail a tutti i processi*/
            MPI_Bcast(fail, m, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
    }

    int i=0;
    int check=1;
    int count=0;

    /*elaborazione del file in input a blocchi di massimo 50MB*/
    while(check!=0)
    {
            /*MPI_Barrier(MPI_COMM_WORLD);*/
            i=0;
            size_text=SIZE;

            if (rank==0)
            {
                   text = malloc(sizeof(char)*size_text);

                    if (count!=0)
                    {
                            offset=count*MAX_BUFF_SIZE-m+1;
                    }

                    /*acquisizione del testo in cui cercare il pattern*/
                    lseek(fp,offset,SEEK_SET);

                    hpmStartx( 1+(count*2+1), HPM_ONLY_EXCLUSIVE, "Acquisizione dell'input");
                    if (hpm_error_count) exit(4);

                    while (i<MAX_BUFF_SIZE && check!=0)
                    {
                            if(i==size_text)
                            {
                                    size_text=size_text<<1;
                                    text=realloc(text, sizeof(char)*size_text);
                            }
                            if (text == NULL)
                            {
                                    printf("\nMemoria insufficiente\n");
                            }

                            check=read(fp, &(text[i]), sizeof(char));
                            i=i+1;
                    }/*fine while acquisizione testo per un blocco*/


                    hpmStop( 1+(count*2+1));
                    if (hpm_error_count) exit(4);


                    size_text=(!check)? (i-1):i;/*size_text corrisponde al numero di caratteri del testo in input*/
                    text=realloc(text, sizeof(char)*(size_text+1));
                    text[size_text]=0;

                    /*evitare la computazione dell'algoritmo se i parametri in input non sono logicamente corretti*/
                    if(size_text<m)
                    {
                            printf("Il pattern non è presente all'interno del testo\n");
                            return 0;
                    }

                    if (size>1)
                    {

                        /*lunghezza della stringa divisa tra i vari processori nel primo ciclo (testo - ultimi (m-1) caratteri)*/
                        int true_size=size_text-m+1;
                        /*numero di caratteri che ogni processo andrà ad analizzare (ad esclusione dell'ultimo)*/
                        rank_size=true_size/size;
                        /*numero di caratteri da aggiungere a quelli che analizza l'ultimo processo*/
                        last_extra=true_size%size;
                        /*indice del primo carattere da leggere (inizializzata ad un valore non valido)*/
                        int begin_r=-1;
                        /*indice della posizione in cui scrivere il primo carattere di una stringa del secondo ciclo(inizializzata ad un valore non valido)*/
                        int begin_w=-1;
                        /*numero di caratteri totali che ogni processo andrà ad analizzare nel secondo ciclo*/
                        size_cycle2=size*(length_string2)+1;

                        text2=malloc(sizeof(char)*size_cycle2);

                        for(i=size-1; i>0; i--)
                        {
                                int k=size-1-i;

                                begin_r=((k+1)*rank_size)-m+1;

                                /*memorizzazione degli indici del primo carattere passato successivamente ai processori per il secondo ciclo*/
                                indices[k]=begin_r;

                                /*acquisizione del testo necessario per il secondo ciclo*/
                                begin_w=k*length_string2;

                                int j;
                                for(j=length_string2; j--; )
                                {
                                        text2[begin_w+j]=text[begin_r+j];
                                }
                        }

                        /*acquisizione degli ultimi caratteri del testo necessari per il secondo ciclo*/
                        begin_r=size_text-length_string2-1;
                        indices[size-1]=begin_r;

                        begin_w=(size-1)*length_string2;
                        int j=0;

                        for(j=length_string2; j--; )
                        {
                                text2[begin_w+j]=text[begin_r+j];
                        }
                        text2[size_cycle2-1]=0;

                        /*dati MPI usata per la comunicazione punto a punto successiva*/
                        MPI_Request req;
                        /*invio del numero di caratteri da aggiungere a quelli da analizzare all'ultimo processo*/
                        MPI_Isend(&last_extra, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD, &req);
                    }

                }/*fine if(rank==0)*/

                /*
                indice del primo carattere della stringa, passata al processore
                nel secondo ciclo dell'algoritmo, rispetto al testo originale
                */
                int index=-1;

                if (size>1)
                {
                        MPI_Bcast(&offset, 1, MPI_INT, 0, MPI_COMM_WORLD);
                        MPI_Barrier(MPI_COMM_WORLD);

                        /*invio della dimensione del buffer da allocare in ogni processo per ricevere il testo da analizzare*/
                        MPI_Bcast(&rank_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
                        MPI_Barrier(MPI_COMM_WORLD);


                        /*dichiarazione della grandezza della stringa che verrà passata a ciascun processore nel primo ciclo*/
                        int rcv_size=rank_size;
                        MPI_Status stat;
                        if (rank==size-1)
                        {

                                /*ricezione dell'ultimo processo del numero di caratteri in più da leggere rispetto agli altri processi*/
                                MPI_Recv(&last_extra, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &stat);
                                rcv_size=rcv_size+last_extra;

                        }

                        /*stringa analizzata da ciascun processo al primo ciclo*/
                        rcv_buff=malloc(sizeof(char)*(rcv_size+1));

                        /*stringa analizzata da ciascun processo al secondo ciclo*/
                        rcv_buff2=malloc(sizeof(char)*((m<<1)-1));
                        MPI_Barrier(MPI_COMM_WORLD);

                        /*passaggio di rank_size caratteri a ciascun processore (verranno analizzati al primo ciclo)*/
                        MPI_Scatter(text, rank_size, MPI_CHAR, rcv_buff, rank_size, MPI_CHAR, 0, MPI_COMM_WORLD);
                	    MPI_Barrier(MPI_COMM_WORLD);

                        /*
                        passaggio dei restanti caratteri necessari all'ultimo processore (last_extra)
                        per definire la stringa che analizzerà nel primo ciclo
                        */
                        if(rank==0)
                        {
                                /*dati MPI usata per la comunicazione punto a punto successiva*/
                                MPI_Request req;
                                MPI_Isend(&text[size*rank_size], last_extra, MPI_CHAR, size-1, 1, MPI_COMM_WORLD, &req);

                        }
                        else if(rank==size-1)
                        {
                                MPI_Recv(&rcv_buff[rank_size], last_extra, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);
                        }
                	    rcv_buff[rcv_size] = 0;

                        /*comunicazione a tutti i processi delle stringhe da analizzare nel secondo ciclo*/
                        MPI_Scatter(text2, length_string2, MPI_CHAR, rcv_buff2, length_string2, MPI_CHAR, 0, MPI_COMM_WORLD);

                        /*comunicazione a tutti i processi della lunghezza del testo*/
                        MPI_Bcast(&size_text, 1, MPI_INT, 0, MPI_COMM_WORLD);

            	       /*
                       passaggio dell'indice trovare il primo carattere del secondo ciclo nel testo complessivo
                       (necessario per il calcolo dell'indice assoluto di inizio dell'occorenza del pattern)
                       */
                       MPI_Scatter(indices, 1, MPI_INT, &index, 1, MPI_INT, 0, MPI_COMM_WORLD);
                       MPI_Barrier(MPI_COMM_WORLD);

                }

                /*
                #########################################################
                #                Algoritmo KMP parallelo                #
                #########################################################
                */

                hpmStartx( 2+(count*2+1), HPM_ONLY_EXCLUSIVE, "Algoritmo KMP");
                if (hpm_error_count) exit(4);

                /*Calcolo dell'indice della prima occorrenza nella stringa passata al processore nel Ciclo 1*/
                int partial_result;

                if (size==1)
                {
                        result=findKMP(text,pattern,m,fail);
                        if (result!=-1)
                        {
                                if (count!=0)
                                {
                                        result=result+offset;
                                }
                                check=0;
                        }
                }
                else
                {
                        partial_result= findKMP(rcv_buff, pattern, m, fail);

                        if(partial_result!=-1)
                        {
                                /*
                                --------------Ciclo 1------------------------
                                Calcolo dell'indice della prima occorrenza del Ciclo 1 all'intero file di testo
                                */
                                partial_result=partial_result+(rank_size*rank);
                        }
                	    else
                        {
                                /*
                                --------------Ciclo 2------------------------
                                Calcolo dell'indice della prima occorrenza nella stringa passata al processore nel Ciclo 2
                                */
                                partial_result= findKMP(rcv_buff2, pattern, m, fail);
                                if(partial_result!=-1)
                                {
                                        /*calcolo dell'indice della prima occorrenza del Ciclo 2 all'intero file di testo*/
          		                        partial_result=partial_result+index;
                                }
                                else
                                {
                                        partial_result=size_text;
                                }
                        }
  	             }
                 hpmStop( 2+(count*2+1) );
                 if (hpm_error_count) exit(4);

                 if (size>1)
                 {

                        MPI_Barrier(MPI_COMM_WORLD);
                        /*comunicazione dell'indice minimo tra quelli valutati da ciascun processore*/
                        MPI_Reduce(&partial_result, &result, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
                        MPI_Barrier(MPI_COMM_WORLD);

                        if(rank==0)
                        {
                                if (result!=size_text)
                                {
                                        if (count!=0)
                                        {
                                                result=result+offset;
                                        }
                                        check=0;
                                }
                                else
                                {
                                        result=-1;
                                }
                        }

                        MPI_Bcast(&check, 1, MPI_INT, 0, MPI_COMM_WORLD);
                        MPI_Barrier(MPI_COMM_WORLD);

                        free(rcv_buff);
                        free(rcv_buff2);
                }
                if (rank==0)
                {
                        free(text);
                        free(text2);
                }
                if (size>1)
                {
                        MPI_Barrier(MPI_COMM_WORLD);
                }

                count=count+1;
    }


    /*stampa dell'indice trovato*/
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

    free(indices);
    free(fail);

    if (rank==0)
    {
            /*Chiusura del file aperto in lettura*/
            close(fp);
    }

    /*Termine di MPI*/
    MPI_Finalize();


    hpmStop( 1 );
    if (hpm_error_count) exit(4);


    hpmTerminate( 0 );
    if (hpm_error_count) exit(4);

    return 0;
}
