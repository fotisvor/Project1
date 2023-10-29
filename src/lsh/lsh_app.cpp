#include <iostream>
#include <string>
#include <cstdint>
#include <chrono>
#include <sys/time.h>

#include "../../include/io_utils/cmd_args.h"
#include "../../include/io_utils/io_utils.h"
#include "../../include/modules/lsh/lsh.h"
#include "../../include/modules/exact_nn/exact_nn.h"


#define C 1.2

static void start_lsh_simulation(Lsh_args *args) {
    // Create LSH structure
    const uint16_t L = args->get_hash_tables_num();
    const uint16_t N = args->get_nearest_neighbors_num();
    const uint32_t K = args->get_k();
    const double R = args->get_radius();

    std::vector<std::vector<uint8_t>> dataset;
    read_file<uint8_t>(args->get_input_file_path(), dataset);
    const double r = mean_nn_distance<uint8_t>(dataset);

    LSH<uint8_t> lsh(L, N, K, r, dataset);

    while (true) {
        std::vector<std::vector<uint8_t>> queries;
        read_file<uint8_t>(args->get_query_file_path(), queries);
        const size_t num_queries = queries.size();

        std::vector<std::vector<std::pair<uint32_t, size_t>>> ann_results(num_queries, std::vector<std::pair<uint32_t, size_t>>(N));
        std::vector<std::vector<uint32_t>> enn_distances(num_queries, std::vector<uint32_t>(N));
        std::vector<std::vector<size_t>> range_results(num_queries);
        std::vector<std::chrono::microseconds> ann_query_times(num_queries);
        std::vector<std::chrono::microseconds> enn_query_times(num_queries);

        for (size_t i = 0; i < num_queries; ++i) {
            const auto& query = queries[i];

            auto start = std::chrono::high_resolution_clock::now();
            ann_results[i] = lsh.approximate_k_nn(query);
            auto stop = std::chrono::high_resolution_clock::now();
            ann_query_times[i] = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

            start = std::chrono::high_resolution_clock::now();
            enn_distances[i] = exact_nn<uint8_t>(dataset, query, N);
            stop = std::chrono::high_resolution_clock::now();
            enn_query_times[i] = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

            range_results[i] = lsh.approximate_range_search(C, R, query);
        }

        print_statistics(N, num_queries, ann_results, ann_query_times, enn_distances, enn_query_times);

        write_output(args->get_output_file_path(), N, num_queries, ann_results, ann_query_times, enn_distances, enn_query_times, range_results, "LSH");

        std::cout << "You can now open the output file and see its contents" << std::endl;

        if (user_prompt_exit("\nDo you want to continue the simulation and repeat the search process?: [Y/N]: ") != "Y") {
            break;
        }

        if (user_prompt_exit("\nDo you want to use the same query file?: [Y/N]: ") != "Y") {
            args->set_query_file_path(user_prompt_file("\nEnter the path to the new query file: "));
        }
    }
}



int main (int argc, char *argv[]) {
    Lsh_args *args = nullptr;

    if (argc == 15) {
        lsh_parse_args(argc, argv, &args);
    } else if (argc == 1) {
        user_interface(&args);
    } else {
        lsh_usage(argv[0]);
        return EXIT_FAILURE;
    }

    start_lsh_simulation(args);

    delete args;
    return EXIT_SUCCESS;
}
