/* 
El resultado de la ejecucion brindara una segmentacion de una imagen:

La segmentacion divide una imagen en regiones con propiedades internas coherentes. Se puede segmentar una imagen utilizando el color. Donde el objetivo con el
que se agrupan los pıxeles es para separar los elementos significativos de una imagen
y ası poder extraer cierta informacion de alguno de ellos.

*/
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

    // 1. El nodo 0 posee los datos y asigna puntos N/P a cada nodo. Los puntos restantes se asignan uno por uno.
    // 2. El nodo 0 lee y establece los par´ametros de configuracion inicial
    // 3. El nodo 0 elige K puntos como centroides iniciales y los transmite a los otros nodos
    node.scatterDataset();
    node.extractCluster();
    lastIteration = 0;

    /* 
       4. Cada nodo:
            Para cada punto local, busque la pertenencia al grupo entre los K grupos
            El calculo de la distancia entre puntos y centroides se realiza en paralelo
            con OpenMP. 
            Para cada grupo, suma los valores de las dimensiones de los puntos
    */
    for (int it = 0; it < node.getMaxIterations(); it++) {
        

        int notChanged = node.run(it);

        if(notChanged == numNodes){

            lastIteration = it;
            
            break;
        }

        lastIteration = it;
        /*
        5. Despues de una operacion MPI Allreduce, cada nodo conoce el numero de
        puntos y la suma de sus valores dentro de cada grupo. Calcular nuevos centroides.
        */    
    }
    // 6. Se continuara la iteracion desde el punto 4 hasta que finalice.

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