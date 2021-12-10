#include "Kmeans.h"

int Kmeans::getIdNearestCluster(Point p) {
    int idCluster = 0; 

    double sum = 0.0;
    double min_dist;

    sum = squared_norm(clusters[0], p);

    min_dist = sqrt(sum);

    for (int k = 1; k < K; k++) {
        sum = 0.0;
        double dist;
        dist = euclidianDistance(sum);

        if (dist < min_dist) {
            min_dist = dist;
            idCluster = k;
        }
    }


    return idCluster;
}

double Kmeans::euclidianDistance(Point a,Point b){
    double distance = 0;

    for(int j = 0; j < a.maxDim; j++){
        distance += pow(a.position[j] - b.position[j], 2.0);
    }

    distance = sqrt(distance);

    return distance;
}
