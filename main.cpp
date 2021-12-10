#include <iostream>
#include <mpi.h>
#include "Worker.h"

#define MAX_ITE 1000

int main(int argc, char *argv[]) {
    int numWokers, rank;
    
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);

    Worker worker(rank);
    
    worker.scatterPoints(); //  each worker should have n points ( n = totalPoints / totalWorkers)
    worker.broadcastCentroids();

    for(int i=0;i< MAX_ITE;i++){
        worker.nextIteration();
    }


    MPI_Finalize();

    return 0;
}