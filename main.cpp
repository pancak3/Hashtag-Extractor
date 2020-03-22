// Entry point of program
// MPI divides work here

// References:
// https://stackoverflow.com/questions/14718124
// https://stackoverflow.com/questions/5122804
// https://stackoverflow.com/questions/31323135
// Note that comments/code may be adapted from man pages

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>
#include "process_section.hpp"
#include "retriever.hpp"

using std::pair;
using std::string;
using std::unordered_map;

// Prototypes
long long get_file_length(const char *filename);

void perform_work(const char *filename, long long file_length,
                  unordered_map<string, string> &country_codes);

unordered_map<string, string> read_country_csv(const char *filename);

void receive_content_to_res(unordered_map<string, unsigned long>
                            &res, int
                            from_rank);

void combine(pair<unordered_map<string, unsigned long>,
        unordered_map<string, unsigned long>>
             results,
             int rank, int size);

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " "
                  << "input.json country_codes.csv" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Init executation environment
    MPI::Init(argc, argv);

    // Get number of bytes in file
    long long file_length = get_file_length(argv[1]);

    // Read country code CSV
    // Assuming that there's not much overhead in reading a small file...
    std::unordered_map<string, string> country_codes =
            read_country_csv(argv[2]);

    // Split and perform work
    perform_work(argv[1], file_length, country_codes);

    // Terminates MPI execution environment
    MPI::Finalize();

    return 0;
}

// Determine work done by each node and joins results
void perform_work(const char *filename, const long long file_length,
                  unordered_map<string, string> &country_codes) {
    int rank, size;

    // Get rank of current communicator + size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Print file size
    if (rank == 0) {
        std::stringstream m;
        m << "File size (in bytes): " << file_length << std::endl;
        std::cerr << m.str();
    }

    // Let's start off with even divisions and improve it if we have time
    // Note: start and end are inclusive
    long long chunk = file_length / size + (file_length % size == 0 ? 0 : 1);
    long long start = rank * chunk;
    long long end = std::min(file_length, (rank + 1) * chunk - 1);
    if (rank == size - 1) {
        end = file_length - 1;
    }

    // Print chunks allocated
    std::stringstream m;
    m << "rank " << rank << ", start: " << start << ", end: " << end
      << std::endl;
    std::cerr << m.str();

    // For the current process, divide the work further (into threads)
    // Purpose: though we can have 1 MPI process for each core, let's try to
    // avoid network communication overheads
    pair<unordered_map<string, unsigned long>,
            unordered_map<string, unsigned long>>
            results = process_section(filename, start, end);

    // Combine and print results
    combine(results, rank, size);
}

void send_combined_data(unordered_map<string, unsigned long> combined_res) {
    string content;
    int content_len = 0;
    int freq[combined_res.size()];

    unordered_map<string, unsigned long>::iterator j;
    int count = 0;
    for (j = combined_res.begin(); j != combined_res.end(); j++) {
        content += j->first + ' ';
        content_len += j->first.length() + 1;
        freq[count] = j->second;
        count++;
    }
    // Send length
    MPI_Send(&content_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(&content[0], content_len, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
    MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(freq, count, MPI_INT, 0, 2, MPI_COMM_WORLD);

}

void receive_content_to_res(unordered_map<string, unsigned long>
                            &res, int
                            from_rank) {
    //receive hash tags content
    int content_len;
    MPI_Recv(&content_len, 1, MPI_INT, from_rank, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    char content[content_len];
    MPI_Recv(content, content_len, MPI_CHAR, from_rank, 1,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    //receive hash tag freq
    int freq_len;
    MPI_Recv(&freq_len, 1, MPI_INT, from_rank, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    int freq[freq_len];
    MPI_Recv(freq, freq_len, MPI_INT, from_rank, 2,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    string key;
    int count = 0;
    for (int l = 0; l < content_len; ++l) {
        if (content[l] != ' ') {
            key.push_back(content[l]);
        } else {
            if (res.end() !=
                res.find(key)) {
                res[key] += freq[count];
            } else {
                res[key] = freq[count];
            }
            key = "";
            count++;
        }
    }

}

void combine(pair<unordered_map<string, unsigned long>,
        unordered_map<string, unsigned long>>
             results,
             int rank, int size) {
    unordered_map<string, unsigned long> combined_lang_freq = results.first;
    unordered_map<string, unsigned long> combined_hashtag_freq =
            results.second;
    unordered_map<string, unsigned long>::iterator j;

    // MPI gather results
    if (rank == 0) {

        for (int i = 1; i < size; i++) {
            receive_content_to_res(combined_hashtag_freq, i);
            receive_content_to_res(combined_lang_freq, i);
        }

#ifdef DEBUG

        std::cout << "[*] Received all." << std::endl;
#endif

    } else {
        // Send hash tag frequencies
        send_combined_data(combined_hashtag_freq);
        // Send lang frequencies
        send_combined_data(combined_lang_freq);

#ifdef DEBUG
        std::cout << "[*] Rank done: " << rank << std::endl;
#endif
    }

    if (rank == 0) {
        std::cout << "[*] HashTag Freq Results" << std::endl;

        for (j = combined_hashtag_freq.begin();
             j != combined_hashtag_freq.end(); j++) {
            std::cout << j->first << " : " << j->second << std::endl;
        }

        std::cout << "[*] Language Freq Results" << std::endl;

        // Add combined results to vector as pairs for sorting
        std::vector<pair<string, unsigned long>> lang_pairs(
                combined_lang_freq.begin(), combined_lang_freq.end());
        std::sort(
                lang_pairs.begin(), lang_pairs.end(),
                [](pair<string, unsigned long> &a,
                   pair<string, unsigned long> b) {
                    return a.second > b.second;
                });

        // Get frequency of 10th element
        unsigned long freq = lang_pairs[9].second;

        // Print out up to 10th element and any ties for 10th place
        for (int i = 0; lang_pairs[i].second >= freq; i++) {
            std::cout << lang_pairs[i].first << " " << lang_pairs[i].second
                      << std::endl;
        }

        long long line_count = 0;
        for (j = combined_lang_freq.begin(); j != combined_lang_freq.end();
             j++) {
            line_count += j->second;
        }
        std::cout << "Total: " << line_count << std::endl;
    }
}

// Gets length of file
long long get_file_length(const char *filename) {
    struct stat sb;
    if (lstat(filename, &sb) == -1) {
        perror("lstat");
        std::exit(EXIT_FAILURE);
    }
    return sb.st_size;
}

// Reads csv file of country codes into <code, country> pairs
unordered_map<string, string> read_country_csv(const char *filename) {
    unordered_map<string, string> country_codes;
    std::ifstream is(filename, std::ifstream::in);
    if (is.fail()) {
        std::cerr << "Cannot access country file" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    string line;
    while (is.good()) {
        // Read line
        getline(is, line);

        // Split and insert
        size_t pos = line.rfind(",");
        country_codes[line.substr(pos + 1, line.length())] =
                line.substr(0, pos);
    }
    is.close();
    return country_codes;
}
