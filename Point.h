#ifndef POINT_H
#define POINT_H

#define MAX_DIM 4 // RGBA

struct Point {
    double position[MAX_DIM];
    int maxDim;      
};

#endif