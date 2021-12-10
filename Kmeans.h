#ifndef KMEANS_H
#define KMEANS_H


#include "Point.h"
#include <vector>

class Kmeans{
    private:
        
        std::vector<Point> centroids;
        std::vector<int> clusters;
        std::vector<Point> localData;
        std::vector<Point> minorDistance;

        int getIdNearestCentroid(Point p); 
        double euclidianDistance(Point a,Point b);

    public:
        void updateMinorDistance();  

};  

#endif