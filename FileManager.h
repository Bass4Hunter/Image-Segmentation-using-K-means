#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include<string>

class FileManager {

public:
    FileManager(int numClusters, int maxIteration, std::string filename,std::string fileimg);
    void createPointsFile();

private:
    int numPoints, pointDimension, numClusters, maxIteration;
    std::string filename;
    std::string fileimg;
};

#endif 