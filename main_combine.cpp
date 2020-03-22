#include <algorithm>
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
		MPI_Send(&count, 1, MPI::UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&length, 1, MPI::UNSIGNED_LONG, 0, 1, MPI_COMM_WORLD);
		MPI_Send(&lang_frequencies[0], count, MPI::UNSIGNED_LONG, 0, 2,
				 MPI_COMM_WORLD);
		MPI_Send(combined.str().c_str(), length, MPI_CHAR, 0, 3,
				 MPI_COMM_WORLD);
		return;
	}

	// Rank 0/node 0
	for (int i = 1; i < size; i++) {
		unsigned long count;
		unsigned long length;

		// Receive length/count
		MPI_Recv(&count, 1, MPI::UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, NULL);
		MPI_Recv(&length, 1, MPI::UNSIGNED_LONG, i, 1, MPI_COMM_WORLD, NULL);

		// Receive arrays
		unsigned long* freq =
			(unsigned long*)malloc(sizeof(unsigned long) * count);
		char* codes = (char*)malloc(sizeof(char) * (length + 1));
		if (codes == NULL || freq == NULL) {
			std::cerr << "Malloc failure" << std::endl;
			std::exit(1);
		}
		MPI_Recv(freq, count, MPI::UNSIGNED_LONG, i, 2, MPI_COMM_WORLD, NULL);
		MPI_Recv(codes, length, MPI_CHAR, i, 3, MPI_COMM_WORLD, NULL);
		codes[length] = '\0';

		// Add frequencies of node to map
        char* code = strtok(codes, ",");
		for (unsigned long f = 0; f < count; f++) {
			if (lang_freq.end() != lang_freq.find(code)) {
				lang_freq[code] += freq[f];
			} else {
				lang_freq[code] = freq[f];
			}
			code = strtok(NULL, ",");
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
