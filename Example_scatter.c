#include <mpi.h>
#include <stdio.h>
#define SIZE 4

int main(int argc, char **argv)
{
    int numtasks,rank,sendcount,recvcount,source;
    float **sendbuf=0;
    float recvbuf[SIZE];
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

    if(numtasks==SIZE){
        source=1;

        if(rank==source){
            sendbuf=loadSquareMatrix(SIZE);
        }

        sendcount=recvcount=SIZE;
        MPI_Scatter(sendbuf,sendcount,MPI_FLOAT,recvbuf,recvcount,
        MPI_FLOAT,source,MPI_COMM_WORLD);
        printf("rank=%dResults:%f%f%f%f\n",rank,recvbuf[0],
        recvbuf[1],recvbuf[2],recvbuf[3]);
    }
    else
        printf("Mustspecify%dprocs.Terminating.\n",SIZE);
    MPI_Finalize();
}
