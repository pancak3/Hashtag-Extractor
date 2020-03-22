// References:
// https://rekols.github.io/2018/04-23/cpp-thousands-separator/

#include <algorithm>
#include <functional>
#include <iterator>
#include <mpi.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

using std::pair;
using std::string;
using std::unordered_map;

void combine_maps(unordered_map<string, unsigned long>& freq_map, int rank,
				  int size);

void easy_print(unordered_map<string, unsigned long>& map,
				std::function<string(string)> printer);

string format_str(string key_str, unordered_map<string, string> country_codes);

// Calls on functions to combines results from multiple nodes together
// and print them
void combine_results(pair<unordered_map<string, unsigned long>,
						  unordered_map<string, unsigned long>>
						 results,
					 int rank, int size,
					 unordered_map<string, string> country_codes) {

	// Extract from pair
	unordered_map<string, unsigned long> combined_lang_freq = results.first;
	unordered_map<string, unsigned long> combined_hashtag_freq =
		results.second;

	// Combine
	combine_maps(combined_lang_freq, rank, size);
	combine_maps(combined_hashtag_freq, rank, size);

	std::function<string(string)> printer =
		std::bind(format_str, std::placeholders::_1, country_codes);
	if (rank == 0) {
		std::cout << std::endl << "[*] Language Freq Results" << std::endl;
		easy_print(combined_lang_freq, printer);
		std::cout << std::endl << "[*] Hashtag Freq Results" << std::endl;
		easy_print(combined_hashtag_freq, printer);
	}
}

string format_str(string key_str,
				  unordered_map<string, string> country_codes) {
	if (key_str[0] == '#') {
		return key_str + ",";
	} else if ('a' <= key_str[0] && key_str[0] <= 'z') {
		return country_codes[key_str] + " (" + key_str + "),";
	} else {
		for (int i = key_str.length() - 3; i >= 1; i -= 3) {
			key_str.insert(i, ",");
		}
		return key_str;
	}
}

// Prints top 10 of <string, unsigned long> maps
// TODO: function pointer for languages
void easy_print(unordered_map<string, unsigned long>& map,
				std::function<string(string)> printer) {
	unordered_map<string, unsigned long>::iterator it;

	// Put items of map into vector as pairs for sorting
	// Adapted from stackoverflow 5122804, 31323135
	std::vector<pair<string, unsigned long>> pairs(map.begin(), map.end());
	std::sort(
		pairs.begin(), pairs.end(),
		[](pair<string, unsigned long>& a, pair<string, unsigned long> b) {
			return a.second > b.second;
		});

	// Get frequency of 10th element
	unsigned long freq = pairs[9].second;

	// Print up to 10th element and any ties for 10th place
	for (int i = 0; pairs[i].second >= freq; i++) {
		std::cout << printer(pairs[i].first) << " "
				  << printer(std::to_string(pairs[i].second)) << std::endl;
	}
}

// Combines maps from multiple nodes together
void combine_maps(unordered_map<string, unsigned long>& freq_map, int rank,
				  int size) {
	// Rank not 0, send results to rank 0
	if (rank != 0) {
		std::vector<string> keys;
		std::vector<unsigned long> frequencies;

		// Elements in map to 2 vectors
		unordered_map<string, unsigned long>::iterator it;
		for (it = freq_map.begin(); it != freq_map.end(); it++) {
			keys.push_back(it->first);
			frequencies.push_back(it->second);
		}

		// Combine keys to comma separated string
		// Adapted from stackoverflow 5689003
		std::stringstream combined;
		std::copy(keys.begin(), keys.end(),
				  std::ostream_iterator<string>(combined, ","));

		// Number of key/value pairs
		unsigned long count = frequencies.size();
		// Length of key string
		unsigned long length = combined.str().length();

		// Send all to first node
		MPI_Send(&count, 1, MPI::UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&length, 1, MPI::UNSIGNED_LONG, 0, 1, MPI_COMM_WORLD);
		MPI_Send(&frequencies[0], count, MPI::UNSIGNED_LONG, 0, 2,
				 MPI_COMM_WORLD);
		MPI_Send(combined.str().c_str(), length, MPI_CHAR, 0, 3,
				 MPI_COMM_WORLD);
		return;
	}

	// Rank 0/node 0
	for (int i = 1; i < size; i++) {
		unsigned long count;
		unsigned long length;

		// Receive length and count
		MPI_Recv(&count, 1, MPI::UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, NULL);
		MPI_Recv(&length, 1, MPI::UNSIGNED_LONG, i, 1, MPI_COMM_WORLD, NULL);

		// Receive arrays
		unsigned long* frequencies =
			(unsigned long*)malloc(sizeof(unsigned long) * count);
		char* keys = (char*)malloc(sizeof(char) * (length + 1));
		if (keys == NULL || frequencies == NULL) {
			std::cerr << "Malloc failure" << std::endl;
			std::exit(1);
		}
		MPI_Recv(frequencies, count, MPI::UNSIGNED_LONG, i, 2, MPI_COMM_WORLD,
				 NULL);
		MPI_Recv(keys, length, MPI_CHAR, i, 3, MPI_COMM_WORLD, NULL);
		keys[length] = '\0';

		// Add frequencies of node to map
		char* code = strtok(keys, ",");
		for (unsigned long f = 0; f < count; f++) {
			if (freq_map.end() != freq_map.find(code)) {
				freq_map[code] += frequencies[f];
			} else {
				freq_map[code] = frequencies[f];
			}
			code = strtok(NULL, ",");
		}

		free(frequencies);
		free(keys);
	}
}
