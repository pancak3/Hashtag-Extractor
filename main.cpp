// Entry point of program
// MPI divides work here

// References:
// https://stackoverflow.com/questions/14718124
// https://stackoverflow.com/questions/5122804
// https://stackoverflow.com/questions/31323135
// https://stackoverflow.com/questions/5689003
// Note that comments/code may be adapted from man pages

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mpi.h>
#include <sstream>
#include <stdlib.h>
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
long long get_file_length(const char* filename);
void perform_work(const char* filename, long long file_length,
				  unordered_map<string, string>& country_codes);
unordered_map<string, string> read_country_csv(const char* filename);
void combine(pair<unordered_map<string, unsigned long>,
				  unordered_map<string, unsigned long>>
				 results,
			 int rank, int size);

int main(int argc, char** argv) {
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
void perform_work(const char* filename, const long long file_length,
				  unordered_map<string, string>& country_codes) {
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
	string send_data;
	int next_msg_len;
	char test[255];
	unordered_map<string, unsigned long>::iterator j;

	for (j = combined_res.begin(); j != combined_res.end(); j++) {
		// store in [lang/#hashtag 12] format
		send_data = j->first + ' ' + std::to_string(j->second);
		// send the end of the string
		next_msg_len = send_data.length() + 1;
		MPI_Send(&next_msg_len, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		MPI_Send(&send_data[0], next_msg_len, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
}

// Prints top 10 of <string, unsigned long> maps
// TODO: function pointer for languages
void print_results(unordered_map<string, unsigned long>& map) {
	unordered_map<string, unsigned long>::iterator it;

	// Add combined results to vector as pairs for sorting
	std::vector<pair<string, unsigned long>> pairs(map.begin(), map.end());
	std::sort(
		pairs.begin(), pairs.end(),
		[](pair<string, unsigned long>& a, pair<string, unsigned long> b) {
			return a.second > b.second;
		});

	// Get frequency of 10th element
	unsigned long freq = pairs[9].second;

	// Print out up to 10th element and any ties for 10th place
	for (int i = 0; pairs[i].second >= freq; i++) {
		std::cout << pairs[i].first << " " << pairs[i].second << std::endl;
	}

	long long line_count = 0;
	for (it = map.begin(); it != map.end(); it++) {
		line_count += it->second;
	}
	std::cout << "Total: " << line_count << std::endl;
}

void combine_lang(unordered_map<string, unsigned long>& lang_freq, int rank,
				  int size) {
	unordered_map<string, unsigned long>::iterator it;

	// Rank not 0, send results to rank 0
	if (rank != 0) {
		std::vector<string> lang_codes;
		std::vector<unsigned long> lang_frequencies;

		// Elements in map to 2 vectors
		for (it = lang_freq.begin(); it != lang_freq.end(); it++) {
			lang_codes.push_back(it->first);
			lang_frequencies.push_back(it->second);
		}

		// Combine lang_codes to comma separated string
		std::stringstream combined;
		std::copy(lang_codes.begin(), lang_codes.end(),
				  std::ostream_iterator<string>(combined, ","));

		// Number of language codes/value pairs
		unsigned long count = lang_frequencies.size();
		// Length of language code string
		unsigned long length = combined.str().length();

		// Send all to first node
		MPI_Send(&count, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&length, 1, MPI_UNSIGNED_LONG, 0, 1, MPI_COMM_WORLD);
		MPI_Send(combined.str().c_str(), length, MPI_CHAR, 0, 2,
				 MPI_COMM_WORLD);
		MPI_Send(&lang_frequencies[0], count, MPI_UNSIGNED_LONG, 0, 3,
				 MPI_COMM_WORLD);

		return;
	}

	// Rank 0/node 0
	for (int i = 1; i < size; i++) {
		unsigned long count;
		unsigned long length;

		// Receive length/count
		MPI_Recv(&length, 1, MPI_UNSIGNED_LONG, i, 1, MPI_COMM_WORLD, NULL);
		MPI_Recv(&count, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, NULL);

		// Receive arrays
		unsigned long* freq =
			(unsigned long*)malloc(sizeof(unsigned long) * count);
		char* codes = (char*)malloc(sizeof(char) * (length + 1));
		if (codes == NULL || freq == NULL) {
			std::cerr << "Malloc failure" << std::endl;
			std::exit(1);
		}
		MPI_Recv(freq, count, MPI_UNSIGNED_LONG, i, 2, MPI_COMM_WORLD, NULL);
		MPI_Recv(codes, length, MPI_CHAR, i, 3, MPI_COMM_WORLD, NULL);
		codes[length] = '\0';

		// Add frequencies of node to map
		for (unsigned long i = 0; i < count; i++) {
			char* code = strtok(codes, ",");
			lang_freq[code] += freq[i];
		}

		free(freq);
		free(codes);
	}

	std::cout << "[*] Language Freq Results" << std::endl;
	print_results(lang_freq);
}

void combine(pair<unordered_map<string, unsigned long>,
				  unordered_map<string, unsigned long>>
				 results,
			 int rank, int size) {
	unordered_map<string, unsigned long> combined_lang_freq = results.first;
	combine_lang(combined_lang_freq, rank, size);


	unordered_map<string, unsigned long> combined_hashtag_freq =
		results.second;
	unordered_map<string, unsigned long>::iterator j;

	// MPI gather results
	if (rank == 0) {
		char receive_data[255];
		int next_msg_len[255];
		int working_worker = size;

		while (working_worker - 1) {
			for (int i = 1; i < size; i++) {
				if (next_msg_len[i] == -1) {
					continue;
				}
				MPI_Recv(&next_msg_len[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD,
						 MPI_STATUS_IGNORE);
				MPI_Recv(receive_data, next_msg_len[i], MPI_CHAR, i, 0,
						 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				string left;
				string right;
				bool flag = true;

				if (receive_data[0] == '#') {
					// Start with "#"
					for (int k = 0; k < next_msg_len[i]; k++) {
						if (receive_data[k] == ' ') {
							flag = false;
							continue;
						}
						if (flag) {
							left.push_back(receive_data[k]);
						} else {
							right.push_back(receive_data[k]);
						}
					}

					int hash_tag_freq = std::stoi(right);

					if (combined_hashtag_freq.end() !=
						combined_hashtag_freq.find(left)) {
						combined_hashtag_freq[left] += hash_tag_freq;
					} else {
						combined_hashtag_freq[left] = hash_tag_freq;
					}
				} else if ('a' <= receive_data[0] && receive_data[0] <= 'z') {
					// Start with lower character
					for (int k = 0; k < next_msg_len[i]; k++) {
						if (receive_data[k] == ' ') {
							flag = false;
							continue;
						}
						if (flag) {
							left.push_back(receive_data[k]);
						} else {
							right.push_back(receive_data[k]);
						}
					}
					int lang_freq = std::stoi(right);

					if (combined_lang_freq.end() !=
						combined_lang_freq.find(left)) {
						combined_lang_freq[left] += lang_freq;
					} else {
						combined_lang_freq[left] = lang_freq;
					}

				} else {
					// otherwise its stop signal

					string stopped_rank;
					for (int k = 0; k < next_msg_len[i]; k++) {
						if (receive_data[k] == ' ')
							break;
						else
							stopped_rank.push_back(receive_data[k]);
					}
#ifdef DEBUG
					std::cout
						<< "[*] Received stop signal: " << stoi(stopped_rank)
						<< std::endl;
#endif
					next_msg_len[stoi(stopped_rank)] = -1;
					working_worker--;
				};
			}
		}
#ifdef DEBUG

		std::cout << "[*] Received all." << std::endl;
#endif

	} else {
		// Send hash tag frequencies
		send_combined_data(combined_hashtag_freq);
		// Send lang frequencies
		send_combined_data(combined_lang_freq);
		// Send stop message
		string send_data;
		int msg_len;
		send_data = std::to_string(rank);
		send_data.push_back(' ');
		msg_len = send_data.length();
#ifdef DEBUG
		std::cout << "[*] Send stop signal: " << send_data << std::endl;
#endif
		MPI_Send(&msg_len, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		MPI_Send(send_data.c_str(), msg_len, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		std::cout << "[*] HashTag Freq Results" << std::endl;

		for (j = combined_hashtag_freq.begin();
			 j != combined_hashtag_freq.end(); j++) {
			// std::cout << j->first << " : " << j->second << std::endl;
		}
	}
}

// Gets length of file
long long get_file_length(const char* filename) {
	struct stat sb;
	if (lstat(filename, &sb) == -1) {
		perror("lstat");
		std::exit(EXIT_FAILURE);
	}
	return sb.st_size;
}

// Reads csv file of country codes into <code, country> pairs
unordered_map<string, string> read_country_csv(const char* filename) {
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
