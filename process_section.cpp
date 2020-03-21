// Work for one process is divided into threads
// Division into threads, file reading and joining are managed here

// References:
// http://www.cplusplus.com/reference/fstream/ifstream/ifstream/
// https://stackoverflow.com/questions/823479

#include "process_section.hpp"
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <unordered_map>
#include "retriever.hpp"

using std::ifstream;
using std::string;
using std::unordered_map;

// Work size (maximum length of file given to thread)
static const long long CHUNK_SIZE = 1024 * 1024 * 100;

void process_section_thread(ifstream& is, long long start, long long end,
							unordered_map<string, int>& lang_freq_map,
							unordered_map<string, int>& hashtag_freq_map);

// Further subdivides the section [start, end] and assign them to threads
void process_section(const char* filename, long long start, long long end,
					 int rank, int num_worker) {
	// Final combined results
	unordered_map<string, int> combined_lang_freq, combined_hashtag_freq;
	unordered_map<string, int>::iterator j;

	// Again, split into chunks
	// 100 MB chunks for now, note that chunk num_worker cannot be less the
	// shortest line's length
	long long total = (end - start) + 1;
	long long n_chunks =
		total / CHUNK_SIZE + (total % CHUNK_SIZE == 0 ? 0 : 1);

#pragma omp parallel
	{
		// Init maps for each thread
		unordered_map<string, int> lang_freq_map({}), hashtag_freq_map({});
		// Open file for each thread
		ifstream is(filename, std::ifstream::in);

#pragma omp for
		for (long long i = 0; i < n_chunks; i++) {
			// Get chunk start/end
			long long inner_start = start + i * CHUNK_SIZE;
			long long inner_end = start + (i + 1) * CHUNK_SIZE - 1;
			if (inner_end > end) {
				inner_end = end;
			}

			// Process
			process_section_thread(is, inner_start, inner_end, lang_freq_map,
								   hashtag_freq_map);
		}
		is.close();

		// Combine it together
#pragma omp critical
		{
			// Hashtag
			for (j = hashtag_freq_map.begin(); j != hashtag_freq_map.end();
				 j++) {
				if (combined_hashtag_freq.end() !=
					combined_hashtag_freq.find(j->first)) {
					combined_hashtag_freq[j->first] += j->second;
				} else {
					combined_hashtag_freq[j->first] = j->second;
				}
			}

			// Lang
			for (j = lang_freq_map.begin(); j != lang_freq_map.end(); j++) {
				if (combined_lang_freq.end() !=
					combined_lang_freq.find(j->first)) {
					combined_lang_freq[j->first] += j->second;
				} else {
					combined_lang_freq[j->first] = j->second;
				}
			}
		}
	}

	// MPI gather results

	if (rank == 0) {

		char receive_data[255];
		int next_msg_len[255];
		int working_worker = num_worker;

		while (working_worker - 1) {
			for (int i = 1; i < num_worker; i++) {
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

	//#ifdef RESDEBUG
	if (rank == 0) {
		std::cout << "[*] HashTag Freq Results" << std::endl;
		for (j = combined_hashtag_freq.begin();
			 j != combined_hashtag_freq.end(); j++) {
			std::cout << j->first << " : " << j->second << std::endl;
		}

		long long line_count = 0;
		std::cout << "[*] Language Freq Results" << std::endl;
		for (j = combined_lang_freq.begin(); j != combined_lang_freq.end();
			 j++) {
			std::cout << j->first << " : " << j->second << std::endl;
			line_count += j->second;
		}
		std::cout << "Total: " << line_count << std::endl;
	}

	//#endif
}

void send_combined_data(unordered_map<string, int> combined_res) {
	string send_data;
	int next_msg_len;
	char test[255];
	unordered_map<string, int>::iterator j;

	for (j = combined_res.begin(); j != combined_res.end(); j++) {
		// store in [lang/#hashtag 12] format
		send_data = j->first + ' ' + std::to_string(j->second);
		// send the end of the string
		next_msg_len = send_data.length() + 1;
		MPI_Send(&next_msg_len, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		MPI_Send(&send_data[0], next_msg_len, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
}

// Actually process the section [start, end]
void process_section_thread(std::ifstream& is, long long start, long long end,
							unordered_map<string, int>& lang_freq_map,
							unordered_map<string, int>& hashtag_freq_map) {
	char c;
	string line;

	// Print start offset & end offset
	std::stringstream m;
	m << start << " " << end << std::endl;
	std::cerr << m.str();

	// Seek to start
	is.seekg(start);

	// Current position
	long long current = start;

	// Skip first 'line'
	while (is.good()) {
		c = is.get();
		if (c == '\r') {
			c = is.get();
			if (c == '\n') {
				current += 2;
				break;
			}
		}
		current++;
	}

	while (is.good() && current <= end) {
		// Read line
		getline(is, line);
		if (line.length() == 0 && current == end) {
			// At a \n|{"id boundary, need to read/process next line
			// since the thread for next chunk will skip the first line
			continue;
		}
		size_t line_length = line.length();

		// Remove last 2 characters, when applicable to trim to valid json
		// Assumption made that each line ends with r',?\r$'
		if (line[line.length() - 1] == '\r') {
			line.pop_back();
		}
		// First branch should be taken 99.9% of the time
		// Only exception should be last 2 lines
		if (line[line.length() - 1] == ',') {
			line.pop_back();
		} else if (line[line.length() - 1] == ']') {
			// The very last line
			break;
		}

		// Process the line into result
		process_line(line, lang_freq_map, hashtag_freq_map);

		// Increment current by line length and 1 for \n
		current += line_length + 1;
	}
}
