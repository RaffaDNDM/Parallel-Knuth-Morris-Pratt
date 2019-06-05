#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#define SIZE 60

int findKMP(const char *text, const char *pattern, int m, const int *fail);
void computeFailKMP(char * pattern, int m, int *fail);

int main (int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Request *reqs;
  MPI_Status *stats;
  int *size_buff;
  /**
  *************************** RANK 0 ********************************
  */
  int size_text=SIZE;
  int m, *fail;
  char *text,*pattern;
  if (rank==0)
  {
    if(argc<2)
    {
      printf("Mancanti parametri di input (nome_programma filepath pattern)");
      exit(1);
    }

    text = malloc(sizeof(char)*size_text);
    FILE *fp= fopen(argv[1], "r");
    int i=0;

    pattern= argv[2];

    printf("\n pattern %s\n", pattern);
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

    size_text=i-1;
    //size_text=i-1;//tolto l'a capo a mano, ma vedere di fare meglio
    text=realloc(text, sizeof(char)*(size_text));
    //inizio verifica
    text[size_text-1]=0;

    //fine verifica
    printf("\n %s\n", text);
    printf("size_text %d\n", size_text);
    m = strlen(argv[2]);

    fail=malloc(sizeof(int)*m);;
    i=0;
    for(;i<m;i++)
    {
      fail[i]=0;
    }
    computeFailKMP(pattern, m, fail);

    int *size_buff=malloc(sizeof(int)*size);
    int rank_size=(size_text-m+1)/size;
    int last_extra=(size_text-m+1)%size;
    int rcv_size1=0;
    reqs = malloc(sizeof(MPI_Request)*(2*size));
    stats = malloc(sizeof(MPI_Status)*(2*size));
    printf("\n%d-----> %d\n", rank_size, last_extra);

    for(i=0; i<size; i++)
    {
      rcv_size1=rank_size;

      if (i==size-1)
      {
        rcv_size1=rcv_size1+last_extra;
      }
      printf("\nrcv_size1=%d\n",rcv_size1);
      MPI_Isend(&rcv_size1,1,MPI_INT, i, i+size, MPI_COMM_WORLD, &reqs[i+size]);
      printf("\nstep 0: %d\n",reqs[i+size]);
    }
  }
  printf("rank=%d",rank);
  MPI_Barrier(MPI_COMM_WORLD);
  //printf("\nsize_buff[%d]=%d\n",rank, size_buff[rank]);
  //MPI_Irecv(&size_buff[rank],1,MPI_INT, rank, rank, MPI_COMM_WORLD, &reqs[rank]);
  MPI_Irecv(&size_buff[rank],1,MPI_INT, 0, rank, MPI_COMM_WORLD, &reqs[rank]);
  printf("\n2 size_buff[%d]=%d\n",rank, size_buff[rank]);

  printf("\nstep 1: %d\n", rank );

  int d=MPI_Waitall(2*size,reqs, stats);
  printf("\nd: %d\n", d );
  printf("\nstep 2: %d\n", rank );

  char *rcv_buff1=malloc(sizeof(char)*size_buff[rank]);
  printf("\nstep 3: %d\n", rank );
  //passaggio di N/numero_processori elementi a ciascun processore
  MPI_Scatter(text, size_buff[0], MPI_CHAR, rcv_buff1, size_buff[0], MPI_CHAR, 0, MPI_COMM_WORLD);

  //passaggio del resto di elementi all'ultimo processore
  //(nel caso in cui il numero di caratteri del testo non sia multiplo del numero dei processori)
  /*if (rank==0)
  {
      MPI_lsend();
  }**/
  printf("\nrank %d: %ld\n", rank, sizeof(rcv_buff1)/sizeof(rcv_buff1[0]));

  /**
  i=0;
  for(;i<m;i++)
  {
    printf("%d\n",fail[i]);
  }
  */


//  printf("\n%d\n",findKMP(text, pattern, m, fail));

  free(text);
  free(fail);
  MPI_Finalize();
  return 0;
}

int findKMP(const char *text, const char *pattern, int m, const int *fail)
{
  int n = strlen(text);
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

void computeFailKMP(char * pattern, int m, int *fail)
{
   int j=1;
   int k=0;

   while (j<m)
   {
     if (pattern[j]==pattern[k])
     {
       fail[j]=k+1;
       j=j+1;
       k=k+1;
     }
     else if (k>0)
     {
       k=fail[k-1];
     }
     else
     {
       j=j+1;
     }
   }
}
