/* class Worker que representa a cada nodo; brinda la funcion int run(int it)  
que representa la iteracion para obtener el contenido de cada cluster */

#ifndef WORKER_H
#define WORKER_H

#include <string>
#include <mpi.h>
#include <math.h>
#include <vector>
#include "Point.h"

class Worker{

private:
    int rank;
    MPI_Comm comm;
    MPI_Datatype pointType;

    int total_values;
    int num_local_points;
    int K, max_iterations;
    int numPoints;  
    int notChanged; 

    int* memCounter;        
    int lastIteration;
    bool newDatasetCreated;
    std::string newDatasetFilename;
    std::string imgFilename;

    double* reduceArr;
    double* reduceResults;

    std::vector<Point> dataset;
    std::vector<Point> localDataset;
    std::vector<Point> clusters;
    std::vector<Point> localSum;
    int numPointsPerNode;
    std::vector<int> memberships; 
                           
    int* globalMembership;
    double total_time;
    double omp_total_time;


    int getIdNearestCluster(Point p); 
    void updateLocalSum(); 

public:
    Worker(int rank, MPI_Comm comm = MPI_COMM_WORLD);
    ~Worker();
    int getMaxIterations();
    void readDataset();
    void createDataset(std::string fileImg);
    void scatterDataset();
    void extractCluster();
    int run(int it);
    void computeGlobalMembership();
    int getNumPoints();
    int* getGlobalMemberships();
    void writeClusterMembership();
    double squared_norm(Point p1, Point p2);
    double cosine_similarity(Point p1, Point p2);
    void setLastIteration(int lastIt);

};


#endif
