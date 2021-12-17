#include "FileManager.h"
#include <fstream>
#include "jpeg.h"

FileManager::FileManager( int numClusters, int maxIteration, std::string filename,std::string fileimg) {
    marengo::jpeg::Image imgOriginal("data/test.jpg");
    this->numPoints = imgOriginal.getWidth() * imgOriginal.getHeight();
    this->pointDimension = imgOriginal.getPixelSize();
    this->numClusters = numClusters;
    this->maxIteration = maxIteration;
    this->filename = filename;
    this->fileimg = fileimg;
}

void FileManager::createPointsFile(){
    srand(time(NULL));
    std::ofstream myfile;
    marengo::jpeg::Image imgOriginal("data/test.jpg");
    myfile.open("data/" + filename + ".csv");
    myfile << pointDimension << "," << numClusters << "," << maxIteration << "\n";
    for(int i = 0; i < imgOriginal.getHeight(); i++){
        for(int j=0 ;j < imgOriginal.getWidth(); j++){
            std::vector<u_int8_t> pixel = imgOriginal.getPixel(j,i);
            for(int n = 0;n < pointDimension;n++){
                myfile << (int)pixel[n] ;
                if(n < pointDimension-1){
                    myfile << ",";
                }
            }
            myfile << "\n";

        }

    }
    myfile.close();
}

