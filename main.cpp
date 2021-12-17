#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include "Point.h"
#include "Worker.h"
#include <stddef.h>
#include <mpi.h>
#include <fstream>
#include <sstream>
#include "jpeg.h"

using namespace std;
int main(int argc, char *argv[]) {

    srand(time(NULL));

    int numNodes, rank;
    const int tag = 13;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numNodes);

    int total_values;
    int total_points;
    int K, max_iterations;
    int lastIteration;
    vector<Point> dataset;

    Worker node(rank, MPI_COMM_WORLD);

    string fileImg = "data/test.jpg";
    node.createDataset(fileImg);
    node.readDataset();

    node.scatterDataset();
    node.extractCluster();
    lastIteration = 0;
    for (int it = 0; it < node.getMaxIterations(); it++) {

        int notChanged = node.run(it);

        if(notChanged == numNodes){

            lastIteration = it;
            
            break;
        }

        lastIteration = it;
    }

    node.setLastIteration(lastIteration);

    node.computeGlobalMembership();
    
    if(rank == 0){
        int* gm;
        gm = node.getGlobalMemberships();
        int numPoints = node.getNumPoints();
        node.writeClusterMembership();        
    }


    MPI_Finalize();
    return 0;
}