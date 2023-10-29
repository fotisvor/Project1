#include <iostream>
#include <vector>
#include <string>
#include <utility>

#include "../../include/io_utils/io_utils.h"
#include "../../include/cluster/cluster_utils.h"
#include "../../include/cluster/cluster.h"

int main(int argc, char *argv[]) {
    ClusterArgs args;
    ClusterConfigs configs = {};
    Cluster<uint8_t> *cluster = nullptr;

    if (argc != 9 && argc != 10) 
    {
        ClusterUsage(argv[0]);
    }

    ClusterParse(argc, argv, &args);
    ClusterConf(args.config_file, &configs);

    std::vector<std::vector<uint8_t>> trainingDATA;
    std::cout << "\nreading training set from.... \"" << args.InputFile << "\"..." << std::endl;
    read_file<uint8_t>(args.InputFile, trainingDATA);
    std::cout << "all done..!" << std::endl;

    if (args.METHOD == "Classic") 
    {
        cluster = new Cluster<uint8_t>(configs.ClusterNumber);
    } else if (args.METHOD == "LSH") 
    {
        double r = NN_distance<uint8_t>(trainingDATA);
        cluster = new Cluster<uint8_t>(configs.ClusterNumber, configs.HashTable_number, 0,configs.HashFunction_Number, r, trainingDATA);
    } 
    else 
    {
        double r = NN_distance<uint8_t>(trainingDATA);
        cluster = new Cluster<uint8_t>(configs.ClusterNumber, configs.hypercube_dimensions,configs.MAX_mHypercube, configs.ProbesNumber, 0, 0.0,
            trainingDATA.size(), trainingDATA[0].size(), r, trainingDATA);
    }

    std::cout << "\nK-Medians++ is starting..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    cluster->k_medians_plus_plus(trainingDATA, args.METHOD);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop-start);
    std::cout << "all done..!" << std::endl;

    std::cout << "\ncalculating silhouette..."<<std::endl;
    cluster->silhouette(trainingDATA);
    std::cout << "all done..!" << std::endl;
    std::cout << "\nWriting output to \"" << args.OutputFile << "\"....." << std::endl;
    cluster->ClusterOUTput(args.OutputFile, args.METHOD, args.complete, duration);
    std::cout << "all done..!" << std::endl;
    delete cluster;
    return EXIT_SUCCESS;
}
