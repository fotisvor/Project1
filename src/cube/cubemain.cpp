#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sys/time.h>

#include "../../include/modules/hypercube/hypercube.h" 
#include "../../include/io_utils/cmd_args.h"
#include "../../include/io_utils/io_utils.h"
#include "../../include/modules/exact_nn/exact_nn.h"


static void HyperCubeSimulation(CubeArgs *args)
{

    std::vector<std::vector<uint8_t>> TrainSamples;
    std::cout << "\nReading training set from \"" << args->get_InputPath() << "\"...." << std::endl;
    read_file<uint8_t> (args->get_InputPath(),TrainSamples);
    std::cout<<"all Done.."<<std::endl;
    std::cout << "\nComputing mean_nearest_neighbor distance..." <<std::endl;
    double r = NN_distance<uint8_t> (TrainSamples);  
    std::cout << "all Done!"<<std::endl;

    std::cout <<"\nMaking Hypercube structure." <<std::endl;
    auto start=std::chrono::high_resolution_clock::now();
    Hypercube<uint8_t> cube(args->get_K(), args->get_MAXcandidates(), args->get_MAXprobes(), args->get_NearNeighborNum(), args->get_RADIUS(), TrainSamples.size(), \
        TrainSamples[0].size(), r, TrainSamples);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    std::cout << "all Done..." <<std::endl;
    std::cout << "Hypercube Structure making lasted..." << duration.count() << "..seconds" <<std::endl;

    while (1) 
    {

        std::vector<std::vector<uint8_t>> TestSamples;
        std::cout << "\nReading test set from \"" << args->get_QueryPath() << "\"..." << std::endl;
        read_file<uint8_t> (args->get_QueryPath(), TestSamples);
        std::cout << "all Done..." << std::endl;

        std::cout << "\nQuery file contains... " << TestSamples.size() << " queries" << std::endl;
        std::cout << "\nStart Executing ANN-ENN-Range-Search"<<std::endl;
        /********** Start ANN / ENN / Range search **********/
        std::vector<std::vector<std::pair<uint32_t, size_t>>> AnnResults(TestSamples.size(), std::vector<std::pair<uint32_t, size_t>> (args->get_NearNeighborNum()));

        std::vector<std::vector<uint32_t>> ENN_dist(TestSamples.size(),std::vector<uint32_t> (args->get_NearNeighborNum()));

        std::vector<std::vector<size_t>> RangeResults(TestSamples.size());
        std::vector<std::chrono::microseconds> Ann_QueryTimes(TestSamples.size());
        std::vector<std::chrono::microseconds> Enn_QueryTimes(TestSamples.size());

        for (size_t i=0;i!=TestSamples.size(); i++) 
        {
        
            start = std::chrono::high_resolution_clock::now();
            AnnResults[i] = cube.approximateNN(TestSamples[i], TrainSamples);
            stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop-start);
            Ann_QueryTimes[i] = duration;
            
            start = std::chrono::high_resolution_clock::now();
            ENN_dist[i] = exact_nn<uint8_t> (TrainSamples, TestSamples[i], args->get_NearNeighborNum());
            stop = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            Enn_QueryTimes[i] = duration;
        
            RangeResults[i] = cube.range_search(TestSamples[i], TrainSamples);
        }

        PrintStats(args->get_NearNeighborNum(), TestSamples.size(), AnnResults, Ann_QueryTimes, ENN_dist, Enn_QueryTimes);

        std::cout << "\nWriting output to \"" << args->get_OutputPath() << "\"......"<< std::endl;
        write_output(args->get_OutputPath(), args->get_NearNeighborNum(), TestSamples.size(),AnnResults, Ann_QueryTimes, ENN_dist, Enn_QueryTimes, RangeResults, "Hypercube");
        std::cout << "all Done..." << std::endl;

        std::cout << "output file ready to open" <<std::endl;
        std::string option = USER_PromtExit("\nDo you want to continue?: [Y/N]: ");
        if (option == "Y")
        {
            continue;
        }
        else 
        {
            break;
        }
        option = USER_PromtExit("\nDo you want to use the same query_file? [Y/N]: ");
        if(option != "Y") 
        {
            args->set_QueryPath(user_prompt_file("\nEnter path to the new query_file: "));
        }
    }
}


int main(int argc, char *argv[])
{
    CubeArgs *args = nullptr;
    if (argc == 17) 
    {
        Cube_Parse(argc, argv, &args);     
        HyperCubeSimulation(args);
    }
    else if (argc == 1) 
    {                       
        user_interface(&args);
        HyperCubeSimulation(args);
    }
    else 
    {                                      
        CubeUsage(argv[0]);
    }
    delete args;
    return EXIT_SUCCESS;
}
