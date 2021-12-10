#ifndef WORKER_H
#define WORKER_H
class Worker{

    private:
        int rank;
        void allReduceNewCentroids();

    public:
        Node(int rank);
        ~Node();


        void scatterPoints(); //  each worker should have n points ( n = totalPoints / totalWorkers)
        void broadcastCentroids();
        void nextIteration();

};
#endif 