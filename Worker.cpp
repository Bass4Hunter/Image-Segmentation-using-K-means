#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stddef.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include "Worker.h"
#include "FileManager.h"
#include "jpeg.h"
#include <map>
#include <string>

/*Se inicializa las variables que seran compartidas a todos los nodos*/
Worker::Worker(int rank, MPI_Comm comm) : rank(rank), comm(comm), notChanged(1)
{

    int blocksize[] = {MAX_DIM, 1, 1};
    MPI_Aint displ[] = {0, offsetof(Point, id), offsetof(Point, size)};
    MPI_Datatype blockType[] = {MPI_DOUBLE, MPI_INT, MPI_INT};

    MPI_Type_create_struct(3, blocksize, displ, blockType, &pointType);
    MPI_Type_commit(&pointType);
    total_time = 0;
    lastIteration = 0;
    newDatasetCreated = false;
    omp_total_time = 0.0;
}

/* parte del calculo para obtener la distancia euclidiana */
double Worker::squared_norm(Point p1, Point p2)
{
    double sum = 0.0;
    for (int j = 0; j < total_values; j++)
    {
        sum += pow(p1.values[j] - p2.values[j], 2.0);
    }
    return sum;
}

void Worker::setLastIteration(int lastIt)
{
    lastIteration = lastIt;
}

/* utilizando la clase FileManager creamos el archivo csv que contiene todos los puntos que representa los pixeles de una imagen
    y definimos los parametros de la cantidad de clusters (centroides) y la maxima cantidad de iteraciones
    esta opeacion solo se realiza en el nodo 0

*/
void Worker::createDataset(std::string FileImg)
{
    if (rank == 0)
    {
        imgFilename = FileImg;
        std::string answer;

        int numPoints, pointDimension, numClusters, maxIteration;
        std::string filename, out;
        std::cout << "\nNumber of clusters: ";
        getline(std::cin, out);
        numClusters = stoi(out);

        std::cout << "\nMax iteration: ";
        getline(std::cin, out);
        maxIteration = stoi(out);

        newDatasetFilename = "imgPoints";

        FileManager builder(numClusters, maxIteration, newDatasetFilename, FileImg);
        builder.createPointsFile();

        newDatasetCreated = true;
    }
}

/*
    la funcion readDataset permite leer el archivo csv que contiene los puntos totales
    esta opeacion solo se realiza en el nodo 0
*/
void Worker::readDataset()
{

    if (rank == 0)
    {
        std::string filename, point_dimension;

        if (newDatasetCreated)
        {
            filename = "data/" + newDatasetFilename + ".csv";
        }

        std::string distance_choice;
        bool onDistance = true;

        std::ifstream infile(filename);
        std::string line;

        int count = 0;
        int num = 0;
        while (getline(infile, line, '\n'))
        {
            if (count == 0)
            {
                std::stringstream ss(line);
                getline(ss, line, ',');
                total_values = stoi(line);

                getline(ss, line, ',');
                K = stoi(line);

                getline(ss, line, '\n');
                max_iterations = stoi(line);
                count++;
            }
            else
            {
                Point point;
                point.id = num;
                point.size = total_values;
                int i = 0;
                std::stringstream ss(line);
                while (getline(ss, line, ','))
                {
                    point.values[i] = stod(line);
                    i++;
                }
                num++;
                dataset.push_back(point);
            }
        }

        infile.close();

    }
}

/*
    Se realiza un scatter de los puntos totales  (numPoints / numNodes)  a cada nodo de acuerdo a la cantidad de procesadores 
    tambien se realiza un broadcast de la maxima cantidad de iteraciones, y la cantidad de puntos totales.
*/
void Worker::scatterDataset()
{

    double start = MPI_Wtime();

    int numNodes;
    MPI_Comm_size(comm, &numNodes);

    int pointsPerNode[numNodes];
    int datasetDisp[numNodes];

    if (rank == 0)
    {
        numPoints = dataset.size();

        int partial = numPoints / numNodes;
        std::fill_n(pointsPerNode, numNodes, partial);

        if ((numPoints % numNodes) != 0)
        {
            int r = numPoints % numNodes;

            for (int i = 0; i < r; i++)
            {
                pointsPerNode[i] += 1;
            }
        }


        int sum = 0;
        for (int i = 0; i < numNodes; i++)
        {
            if (i == 0)
            {
                datasetDisp[i] = 0;
            }
            else
            {
                sum += pointsPerNode[i - 1];
                datasetDisp[i] = sum;
            }
        }
    }

    MPI_Scatter(pointsPerNode, 1, MPI_INT, &num_local_points, 1, MPI_INT, 0, comm);

    localDataset.resize(num_local_points);

    //Scatter points over the nodes
    MPI_Scatterv(dataset.data(), pointsPerNode, datasetDisp, pointType, localDataset.data(), num_local_points,
                 pointType, 0, comm);

    //Send the dimension of points to each node
    MPI_Bcast(&total_values, 1, MPI_INT, 0, comm);

    memberships.resize(num_local_points);

    for (int i = 0; i < num_local_points; i++)
    {
        memberships[i] = -1;
    }

    MPI_Bcast(&numPoints, 1, MPI_INT, 0, comm);
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, comm);

    double end = MPI_Wtime();
    total_time += end - start;
}


/*la funcion extractCluster crea los centroides (clusters) y realiza  un broadcast de los centroides y la cantidad de centroides*/
void Worker::extractCluster()
{

    if (rank == 0)
    {
        double start = MPI_Wtime();

        if (K >= dataset.size())
        {
            std::cout << "ERROR: Number of cluster >= number of points " << std::endl;
            return;
        }

        std::string string_choice;
        int choice;


        std::vector<int> clusterIndices;
        std::vector<int> prohibitedIndices;

        for (int i = 0; i < K; i++)
        {
            while (true)
            {
                int randIndex = rand() % dataset.size();

                if (find(prohibitedIndices.begin(), prohibitedIndices.end(), randIndex) == prohibitedIndices.end())
                {
                    prohibitedIndices.push_back(randIndex);
                    clusterIndices.push_back(randIndex);
                    break;
                }
            }
        }
        for (int i = 0; i < clusterIndices.size(); i++)
        {
            clusters.push_back(dataset[clusterIndices[i]]);
        }

            
        
        double end = MPI_Wtime();
        total_time += end - start;
    }

    double start_ = MPI_Wtime();

    //Send the number of clusters in broadcast
    MPI_Bcast(&K, 1, MPI_INT, 0, comm);

    clusters.resize(K);

    //Send the clusters centroids values
    MPI_Bcast(clusters.data(), K, pointType, 0, comm);

    double end = MPI_Wtime();

    if (rank != 0)
    {
        total_time += end - start_;
    }
}

/* la funcion getIdNearstCluster permite obtener el cluster de cada punto para esto se utilizo la distancia euclidiana hacia los centroides*/
int Worker::getIdNearestCluster(Point p)
{
    int idCluster = 0; 

    double sum = 0.0;
    double min_dist;

    //Initialize sum and min_dist
    sum = squared_norm(clusters[0], p);

    min_dist = sqrt(sum);

    //Compute the distance from others clusters
    for (int k = 1; k < K; k++)
    {
        sum = 0.0;
        double dist;

        sum = squared_norm(clusters[k], p);
        dist = sqrt(sum);

        if (dist < min_dist)
        {
            min_dist = dist;
            idCluster = k;
        }
    }
    

    return idCluster;
}


/* La funcion run representa la operacion de asignar el cluster de todos los puntos que tiene un nodo 
    y por ultimo se realiza la operacion all reduce para calcular el nuevo  valor de los centroides 
*/
int Worker::run(int it)
{
    double start = MPI_Wtime();
    double t_i, t_f;

    notChanged = 1;
    localSum.resize(K);

    int resMemCounter[K];

    // Reset of resMemCounter at each iteration
    std::fill_n(resMemCounter, K, 0);

    if (it == 0)
    {
        memCounter = new int[K]();
    }

    t_i = omp_get_wtime();
#pragma omp parallel for shared(memCounter)
    for (int i = 0; i < localDataset.size(); i++)
    {

        int old_mem = memberships[i];
        int new_mem = getIdNearestCluster(localDataset[i]);

        if (new_mem != old_mem)
        {
            memberships[i] = new_mem;


//issue: https://stackoverflow.com/questions/17553282/how-to-lock-element-of-array-using-tbb-openmp
#pragma omp atomic update
            memCounter[new_mem]++;

            if (old_mem != -1)
            {
#pragma omp atomic update
                memCounter[old_mem]--;
            }

            notChanged = 0;
        }
    }

    t_f = omp_get_wtime();
    omp_total_time += t_f - t_i;

    MPI_Allreduce(memCounter, resMemCounter, K, MPI_INT, MPI_SUM, comm); // We obtain the number of points that belong to each cluster
    updateLocalSum();

 
    if (it == 0)
    {
        reduceResults = new double[K * total_values];
        reduceArr = new double[K * total_values];
    }

    t_i = omp_get_wtime();
#pragma omp parallel for  shared(reduceArr) 
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < total_values; j++)
        {
            reduceArr[i * total_values + j] = localSum[i].values[j];
        }
    }

    t_f = omp_get_wtime();
    omp_total_time += t_f - t_i;

    MPI_Allreduce(reduceArr, reduceResults, K * total_values, MPI_DOUBLE, MPI_SUM,
                  comm);

    for (int k = 0; k < K; k++)
    {
        for (int i = 0; i < total_values; i++)
        {
            if (resMemCounter[k] != 0)
            {
                reduceResults[k * total_values + i] /= resMemCounter[k];
                clusters[k].values[i] = reduceResults[k * total_values + i];
            }
            else
            {
                reduceResults[k * total_values + i] /= 1;
                clusters[k].values[i] = reduceResults[k * total_values + i];
            }
        }
    }

    int globalNotChanged;

    MPI_Allreduce(&notChanged, &globalNotChanged, 1, MPI_INT, MPI_SUM, comm);

    double end = MPI_Wtime();
    total_time += end - start;

    return globalNotChanged;
}

/* la funcion updateLocalSum permite realizar la sumatoria local  de las distancias de cada cluster
    para facilitar la obtencion de los centroides */

void Worker::updateLocalSum()
{
    //reset LocalSum at each iteration
    for (int k = 0; k < K; k++)
    {
        for (int j = 0; j < total_values; j++)
        {
            localSum[k].values[j] = 0;
        }
    }

    for (int i = 0; i < localDataset.size(); i++)
    {
        for (int j = 0; j < total_values; j++)
        {
            localSum[memberships[i]].values[j] += localDataset[i].values[j];
        }
    }
}

Worker::~Worker()
{
    delete[] reduceArr;
    delete[] reduceResults;
    delete[] memCounter;
    delete[] globalMembership;
}

/*  La funcion computeGlobalMembership permite unir todos los puntos distribuidos, es decir, obtener el cluster de todos los puntos*/
void Worker::computeGlobalMembership()
{

    globalMembership = new int[numPoints];

    int localMem[numPoints];
    int globalMember[numPoints];

    std::fill_n(localMem, numPoints, 0);
    std::fill_n(globalMember, numPoints, 0);

    for (int i = 0; i < num_local_points; i++)
    {
        int p_id = localDataset[i].id;
        int c_id = memberships[i];
        localMem[p_id] = c_id;
    }

    MPI_Reduce(&localMem, &globalMember, numPoints, MPI_INT, MPI_SUM, 0, comm);

    if (rank == 0)
    {
        for (int j = 0; j < numPoints; j++)
        {
            globalMembership[j] = globalMember[j];
        }
    }
}

int *Worker::getGlobalMemberships()
{
    return globalMembership;
}

int Worker::getNumPoints()
{
    return numPoints;
}

int Worker::getMaxIterations()
{
    return max_iterations;
}

struct rgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/* la funcion writeClusterMembership crea un archivo csv que contiene el contenido de cada cluster y 
    tambien crea la imagen segmentada */

void Worker::writeClusterMembership()
{
    std::ofstream myfile;
    myfile.open("results/clusterPoints.csv");
    myfile << "Point_id,Cluster_id"
           << "\n";
    for (int p = 0; p < numPoints; p++)
    {
        myfile << dataset[p].id << "," << clusters[globalMembership[p]].id << "\n";
    }
    myfile.close();

    srand(time(NULL));

    marengo::jpeg::Image SegmentImg(imgFilename);
    int x = 0, y = 0;
    uint8_t r, g, b;
    for (int p = 0; p < numPoints; p++)
    {
        r = clusters[globalMembership[p]].values[0];
        g = clusters[globalMembership[p]].values[1];
        b = clusters[globalMembership[p]].values[2];
        SegmentImg.setPixel(x, y, r, g, b);
        x++;
        if (x == SegmentImg.getWidth())
        {
            x = 0;
            y++;
        }
    }
    SegmentImg.save("results/segmentsImg.jpg");
}
