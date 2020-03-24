// References:
// https://rekols.github.io/2018/04-23/cpp-thousands-separator/

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <mpi.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::pair;
using std::string;
using std::unordered_map;

// Function prototypes
void combine_maps(unordered_map<string, unsigned long>& freq_map, int rank,
				  int size);
void easy_print(unordered_map<string, unsigned long>& map,
				const std::function<string(string)>& printer);
string format_number(string number_str);
string format_lang(unordered_map<string, string> lang_map,
				   const string& short_lang);

/**
 * Calls on functions to combines results from multiple nodes together and
 * print them.
 * @param results pair of lang_freq_map and hashtag_freq_map (pair)
 * @param rank rank of the running process in the group of comm (integer)
 * @param size number of processes in the group of comm (integer)
 * @param lang_map language identifier map e.g.: lang_map["en"] -> "English"
 */
void combine_results(const pair<unordered_map<string, unsigned long>,
								unordered_map<string, unsigned long>>& results,
					 int rank, int size,
					 const unordered_map<string, string>& lang_map) {
	// Extract from pair
	unordered_map<string, unsigned long> combined_lang_freq = results.first;
	unordered_map<string, unsigned long> combined_hashtag_freq =
		results.second;

	// Combine and print
	combine_maps(combined_lang_freq, rank, size);
	combine_maps(combined_hashtag_freq, rank, size);

	std::function<string(string)> lang_printer =
		std::bind(format_lang, lang_map, std::placeholders::_1);
	if (rank == 0) {
		std::cout << std::endl << "[*] Language Freq Results" << std::endl;
		easy_print(combined_lang_freq, lang_printer);
		std::cout << std::endl << "[*] Hashtag Freq Results" << std::endl;
		easy_print(combined_hashtag_freq, [](string key) { return key; });
	}
}

/**
 * Formats a number according to specification.
 * @param number_str string of a number (string), e.g.: "6743991"
 * @return formatted string of the input number (string), e.g.: "6,743,991"
 */
string format_number(string number_str) {
	for (int i = (int)number_str.length() - 3; i >= 1; i -= 3) {
		number_str.insert(i, ",");
	}
	return number_str;
}

/**
 * Formats a language from language identifier to real name.
 * @param lang_map map of <identifier, language> pairs (unordered_map) e.g.:
 * lang_map["en"] -> "English"
 * @param short_lang short name of the language (string) e.g.: "en"
 * @return real name of the short name of a language (string) e.g.: "English"
 */
string format_lang(unordered_map<string, string> lang_map,
				   const string& short_lang) {
	if (lang_map[short_lang].length() == 0) {
		return "Unknown (" + short_lang + ")";
	}
	return lang_map[short_lang] + " (" + short_lang + ")";
}

/**
 * Prints top 10 of <string, unsigned long> maps.
 * @param map combined map of languages or hashtags (unordered_map)
 * @param printer function pointer to format key (pointer)
 */
void easy_print(unordered_map<string, unsigned long>& map,
				const std::function<string(string)>& printer) {
	unordered_map<string, unsigned long>::iterator it;

	// Put items of map into vector as pairs for sorting
	// Adapted from stackoverflow 5122804, 31323135
	std::vector<pair<string, unsigned long>> pairs(map.begin(), map.end());
	std::sort(pairs.begin(), pairs.end(),
			  [](pair<string, unsigned long>& a,
				 const pair<string, unsigned long>& b) {
				  return a.second > b.second;
			  });

	// Get frequency of 10th element
	unsigned long freq = pairs[std::min(9UL, pairs.size() - 1)].second;

	// Print up to 10th element and any ties for 10th place
	for (int i = 0; pairs[i].second >= freq; i++) {
		std::cout << printer(pairs[i].first) << ", "
				  << format_number(std::to_string(pairs[i].second))
				  << std::endl;
	}
}

/**
 * Combine maps from multiple nodes together.
 * @param freq_map frequencies map of languages or hashtags (unordered_map)
 * @param rank rank of the running process in the group of comm (integer)
 * @param size number of processes in the group of comm (integer)
 */
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
		MPI_Send(&frequencies[0], (int)count, MPI::UNSIGNED_LONG, 0, 2,
				 MPI_COMM_WORLD);
		MPI_Send(combined.str().c_str(), (int)length, MPI_CHAR, 0, 3,
				 MPI_COMM_WORLD);
		return;
	}

	// Rank 0/node 0
	for (int i = 1; i < size; i++) {
		unsigned long count;
		unsigned long length;

		// Receive length and count
		MPI_Recv(&count, 1, MPI::UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, nullptr);
		MPI_Recv(&length, 1, MPI::UNSIGNED_LONG, i, 1, MPI_COMM_WORLD,
				 nullptr);

		// Receive arrays
		auto* frequencies =
			(unsigned long*)malloc(sizeof(unsigned long) * count);
		char* keys = (char*)malloc(sizeof(char) * (length + 1));
		if (keys == nullptr || frequencies == nullptr) {
			std::cerr << "Malloc failure" << std::endl;
			std::exit(1);
		}
		MPI_Recv(frequencies, (int)count, MPI::UNSIGNED_LONG, i, 2,
				 MPI_COMM_WORLD, nullptr);
		MPI_Recv(keys, (int)length, MPI_CHAR, i, 3, MPI_COMM_WORLD, nullptr);
		keys[length] = '\0';

		// Add frequencies of node to map
		char* code = strtok(keys, ",");
		for (unsigned long f = 0; f < count; f++) {
			if (freq_map.end() != freq_map.find(code)) {
				freq_map[code] += frequencies[f];
			} else {
				freq_map[code] = frequencies[f];
			}
			code = strtok(nullptr, ",");
		}

		free(frequencies);
		free(keys);
	}
}
