#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>

#include "../../include/cluster/cluster_utils.h"
#include "../../include/io_utils/io_utils.h"

// BEGIN: ed8c6549bwf9 (modified)
void ClusterUsage(const char *exec)
{
    std::cerr << "\nUsage: " << exec << "\n\n"
              << "[+] -i [input_file]\n"
              << "[+] -c [configuration_file]\n"
              << "[+] -o [output_file]\n"
              << "[+] --complete [optional]\n"
              << "[+] -m [assignment method]\n"
              << "\nProvide all the above arguments\n";
    std::exit(EXIT_FAILURE);
}

void ClusterParse(int argc, char *const argv[], ClusterArgs *args)
{

    int opt;
    std::string Input, Output, config;
    int option_i = 0;
    const struct option longopts[] = {{"complete", no_argument, 0, 'f'}, {0, 0, 0, 0}};

    args->complete = false;
    while (true)
    {
        opt = getopt_long(argc, argv, "i:c:o:m:f", longopts, &option_i);
        if (opt == -1)
        {
            break;
        }
        switch (opt)
        {
        case 'i':
            if (!FileExists(optarg))
            {
                std::cerr << "\n[+]Input file does not exist!\n" << std::endl;
                exit(EXIT_FAILURE);
            }
            args->InputFile = optarg;
            break;

        case 'c':
            if (!FileExists(optarg))
            {
                std::cerr << "\n[+]Configuration file does not exist!\n" << std::endl;
                exit(EXIT_FAILURE);
            }
            args->config_file = optarg;
            break;

        case 'o':
            
            if (FileExists(optarg))
            {
                args->OutputFile = optarg;
            }
            else
            {
                std::ofstream out("./Output");
                args->OutputFile = "Output";
            }
            break;

        case 'm':
            args->METHOD = optarg;
            break;

        case 'f':
            args->complete = true;
            break;

        default:

            ClusterUsage(argv[0]);
            break;
        }
    }
}

void ClusterConf(std::string config_file, ClusterConfigs *configs)
{

    std::string Dlimiter = ":";
    std::string Tokenkey;
    size_t position = 0;
    std::ifstream file(config_file);
    std::string line;
    while (std::getline(file, line))
    {
        while ((position=line.find(Dlimiter)) != std::string::npos)
        {
            Tokenkey = line.substr(0, position);
            line.erase(0, position + Dlimiter.length());
        }
        if (Tokenkey == "number of clusters")
        {
            configs->ClusterNumber = stoi(line);
        }
        else if (Tokenkey == "number_of_vector_hash_tables")
        {
            configs->HashTable_number = stoi(line);
            if (configs->HashTable_number == 0)
                configs->HashTable_number = 3;
        }
        else if (Tokenkey == "number_of_vector_hash_functions")
        {
            configs->HashFunction_Number = stoi(line);
            if (configs->HashFunction_Number == 0)
                configs->HashFunction_Number = 4;
        }
        else if (Tokenkey == "max_number_M_hypercube")
        {
            configs->MAX_mHypercube = stoi(line);
            if (configs->MAX_mHypercube == 0)
                configs->MAX_mHypercube = 10;
        }
        else if (Tokenkey == "number_of_hypercube_dimensions")
        {
            configs->hypercube_dimensions = stoi(line);
            if (configs->hypercube_dimensions == 0)
                configs->hypercube_dimensions = 3;
        }
        else if (Tokenkey == "number_of_probes")
        {
            configs->ProbesNumber = stoi(line);
            if (configs->ProbesNumber == 0)
                configs->ProbesNumber = 2;
        }
    }
}


size_t BinarySearch(const std::vector<std::pair<float, size_t>> &PatialSum, float val)
{
    size_t BEGIN = 0;
    size_t END = PatialSum.size();
    while (BEGIN < END)
    {
        size_t middle = BEGIN + (END - BEGIN) / 2;
        if (val < PatialSum[middle].first)
        {
            END = middle;
        }
        else
        {
            BEGIN = middle+1;
        }
    }
    return BEGIN == 0 ? 0 : BEGIN - 1;
}

void NormalizeDist(std::vector<float> &MIN_dist)
{

    float maxdist = std::numeric_limits<float>::min();

    for (float dist : MIN_dist)
    {
        if (dist > maxdist)
            maxdist = dist;
    }

    for (float &d : MIN_dist)
        d /= maxdist;
}

bool in(const std::vector<size_t> &centroid, size_t i)
{
    for (size_t j = 0; j != centroid.size(); ++j)
    {

        if (centroid[j] == i)
            return true;
    }

    return false;
}

bool CompareF(const std::pair<float, size_t> &p1, const std::pair<float, size_t> &p2)
{
    return p1.first < p2.first;
}
