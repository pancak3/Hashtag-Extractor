// Work for one process is divided into threads
// Division into threads, file reading and joining are managed here

// References:
// http://www.cplusplus.com/reference/fstream/ifstream/ifstream/
// https://stackoverflow.com/questions/823479

#define OMPI_SKIP_MPICXX
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <sstream>
#include <string.h>
#include <unordered_map>
#include <utility>
#include "line.hpp"

using std::ifstream;
using std::pair;
using std::string;
using std::unordered_map;

// Prototypes
void process_section_thread(
	ifstream& is, long long start, long long end,
	unordered_map<string, unsigned long>& lang_freq_map,
	unordered_map<string, unsigned long>& hashtag_freq_map);

// Work size (maximum length of file processed by thread at one time)
static const long long CHUNK_SIZE = 1000 * 1000 * 200;

/**
 * Further subdivides the section [start, end], assigns them to threads and
 * combines results.
 * @param filename path of twitter file
 * @param start start byte
 * @param end end byte
 */
pair<unordered_map<string, unsigned long>,
	 unordered_map<string, unsigned long>>
process_section(const char* filename, long long start, long long end) {
	// Final combined results for process
	unordered_map<string, unsigned long> combined_lang_freq,
		combined_hashtag_freq;

	// Further subdivide into chunks of CHUNK_SIZE
	// Note that CHUNK_SIZE cannot be less than length of shortest line
	long long total = (end - start) + 1;
	long long n_chunks =
		total / CHUNK_SIZE + (total % CHUNK_SIZE == 0 ? 0 : 1);

#pragma omp parallel default(none)                                            \
	shared(filename, n_chunks, start, end, combined_lang_freq,                \
		   combined_hashtag_freq, std::cerr, ompi_mpi_comm_world)
	{
		// Init maps (for each thread)
		unordered_map<string, unsigned long> lang_freq_map({}),
			hashtag_freq_map({});
		// Open file (for each thread)
		ifstream is(filename, std::ifstream::in);

		// File reading failure
		if (is.fail() || !is.is_open()) {
			int rank;
			MPI_Comm_rank(MPI_COMM_WORLD, &rank);
			std::cerr << "[!] MPI " << rank << " Thread "
					  << omp_get_thread_num()
					  << " failed to open file, error num:" << strerror(errno)
					  << std::endl;
			std::exit(EXIT_FAILURE);
		}

		// Process chunks in parallel
#pragma omp for ordered
		for (long long i = 0; i < n_chunks; i++) {
			// Determine chunk start/end from chunk index
			long long inner_start = start + i * CHUNK_SIZE;
			long long inner_end = start + (i + 1) * CHUNK_SIZE - 1;
			if (inner_end > end) {
				inner_end = end;
			}

			// Pass work to thread
			process_section_thread(is, inner_start, inner_end, lang_freq_map,
								   hashtag_freq_map);
		}
		is.close();

		// Combine together thread by thread (i.e. not concurrently)
#pragma omp critical
		{
			unordered_map<string, unsigned long>::iterator j;

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

	return pair<unordered_map<string, unsigned long>,
				unordered_map<string, unsigned long>>(combined_lang_freq,
													  combined_hashtag_freq);
}

/**
 * Within each thread, process the section [start, end] by reading line
 * by line and passing each line to the process_line function.
 * process_line then mutates the maps (passed by reference).
 * @param is input stream
 * @param lang_freq_map language frequency map
 * @param hashtag_freq_map hashtag frequency map
 */
void process_section_thread(
	std::ifstream& is, long long start, long long end,
	unordered_map<string, unsigned long>& lang_freq_map,
	unordered_map<string, unsigned long>& hashtag_freq_map) {
	char c;
	string line;

#ifdef DEBUG
	// Print start offset & end offset
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	std::stringstream m;
	m << "[*] MPI " << rank << " Thread " << omp_get_thread_num()
	  << " started work on: " << start << " " << end << std::endl;
	std::cerr << m.str();
#endif

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

		// When applicable, remove 2 characters to trim to valid json
		// Assumption made that each line ends with r',?\r$'
		if (line[line.length() - 1] == '\r') {
			line.pop_back();
		}
		// First branch should be taken 99.9% of the time
		// Only exception should be last 2 lines
		if (line[line.length() - 1] == ',') {
			line.pop_back();
		} else if (line.length() <= 2) {
			// The very last line
			break;
		}

		// Process the line
		process_line(line, lang_freq_map, hashtag_freq_map);

		// Increment current by line_length and 1 for '\n'
		current += line_length + 1;
	}
}
