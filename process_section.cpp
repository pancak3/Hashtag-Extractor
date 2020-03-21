// Work for one process is divided into threads
// Division into threads, file reading and joining are managed here

// References:
// http://www.cplusplus.com/reference/fstream/ifstream/ifstream/
// https://stackoverflow.com/questions/823479

#include <fstream>
#include <iostream>
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
void process_section(char* filename, long long start, long long end) {
	// Final combined results
	unordered_map<string, int> combined_lang_freq, combined_hashtag_freq;
	unordered_map<string, int>::iterator j;

	// Again, split into chunks
	// 100 MB chunks for now, note that chunk size cannot be less the shortest
	// line's length
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

#ifdef RESDEBUG
	std::cout << "[*] HashTag Freq Results" << std::endl;
	for (j = combined_hashtag_freq.begin(); j != combined_hashtag_freq.end();
		 j++) {
		std::cout << j->first << " : " << j->second << std::endl;
	}

	long long line_count = 0;
	std::cout << "[*] Language Freq Results" << std::endl;
	for (j = combined_lang_freq.begin(); j != combined_lang_freq.end(); j++) {
		std::cout << j->first << " : " << j->second << std::endl;
		line_count += j->second;
	}
	std::cout << "Total: " << line_count << std::endl;
#endif
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
