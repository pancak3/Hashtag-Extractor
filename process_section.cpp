// Work for one process is divided into threads
// Division into threads, file reading and joining are managed here

// References:
// https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
// http://www.cplusplus.com/reference/fstream/ifstream/ifstream/
// http://www.cplusplus.com/reference/thread/thread/
// https://thispointer.com/c11-how-to-create-vector-of-thread-objects/
// https://stackoverflow.com/questions/823479
// https://stackoverflow.com/questions/922360

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>
#include "retriever.hpp"

void process_section_thread(char* filename, long long start, long long end,
							unordered_map<string, int>& lang_freq_map,
							unordered_map<string, int>& hashmap_freq_map);

// Further subdivides the section [start, end] and assign them to threads
void process_section(char* filename, long long start, long long end) {
	int fd = open(filename, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		perror("open");
		std::exit(EXIT_FAILURE);
	}

	// Find out how many cores we have available
	unsigned int n = std::thread::hardware_concurrency();

	if (n == 0) {
		// threading not supported
		// simply run base function
		return;
	}

	// Maps for storing frequency counts
	std::vector<unordered_map<string, int>> lang_freq_maps, hashtag_freq_maps;
	for (int i = 0; i < n; i++) {
		// Init map for each thread
		unordered_map<string, int> lang_freq_map({}), hashtag_freq_map({});
		lang_freq_maps.push_back(lang_freq_map);
		hashtag_freq_maps.push_back(hashtag_freq_map);
	}

	// Vec for thread
	std::vector<std::thread> threads;

	// Again, split task into chunks
	long long total = (end - start) + 1;
	long long chunk = total / n + (total % n == 0 ? 0 : 1);
	for (int i = 0; i < n; i++) {
		// Get start/end
		long long thread_start = i * chunk + start;
		long long thread_end = std::min(total, (i + 1) * chunk - 1) + start;
		if (i == n - 1) {
			thread_end = end;
		}

		// Create threads
		threads.push_back(std::thread(
			process_section_thread, filename, thread_start, thread_end,
			std::ref(lang_freq_maps[i]), std::ref(hashtag_freq_maps[i])));
	}

	// Finish up
	for (std::thread& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	// Now combine
	unordered_map<string, int> combined_lang_freq, combined_hashtag_freq;
	unordered_map<string, int>::iterator j;
	for (int i = 0; i < n; i++) {
		// Unwrap the current map
		unordered_map<string, int> hashtag_map = hashtag_freq_maps[i];
		unordered_map<string, int> lang_map = lang_freq_maps[i];

		// Hashtag
		for (j = hashtag_map.begin(); j != hashtag_map.end(); j++) {
			if (combined_hashtag_freq.end() !=
				combined_hashtag_freq.find(j->first)) {
				combined_hashtag_freq[j->first] += j->second;
			} else {
				combined_hashtag_freq[j->first] = j->second;
			}
		}

		// Lang
		for (j = lang_map.begin(); j != lang_map.end(); j++) {
			if (combined_lang_freq.end() !=
				combined_lang_freq.find(j->first)) {
				combined_lang_freq[j->first] += j->second;
			} else {
				combined_lang_freq[j->first] = j->second;
			}
		}
	}

#ifdef RESDEBUG
	cout << "[*] HashTag Freq Results" << endl;
	for (j = combined_hashtag_freq.begin(); j != combined_hashtag_freq.end();
		 j++) {
		cout << j->first << " : " << j->second << endl;
	}

	cout << "[*] Language Freq Results" << endl;
	for (j = combined_lang_freq.begin(); j != combined_lang_freq.end(); j++) {
		cout << j->first << " : " << j->second << endl;
	}
#endif
}

// Actually process the section [start, end]
void process_section_thread(char* filename, long long start, long long end,
							unordered_map<string, int>& lang_freq_map,
							unordered_map<string, int>& hashtag_freq_map) {
	// Print start offset & end offset
	std::stringstream m;
	m << start << " " << end << std::endl;
	std::cerr << m.str();

	// Open file and seek
	char c;
	std::ifstream is(filename, std::ifstream::in);
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

	// Gather frequencies
	string line;

	// TODO: what to do exactly at split...
	while (is.good() && current <= end) {
		// Read line
		getline(is, line);
		if (line.length() == 0) {
			continue;
		}
		size_t line_length = line.length();
		// Remove last 2 characters, if applicable to trim to valid json
		// Assumption made that each line ends with r',?\r$'
		if (line[line.length() - 1] == '\r') {
			line.pop_back();
		}
		if (line[line.length() - 1] == ',') {
			line.pop_back();
		}
		// The very last line
		if (line[line.length() - 1] == ']') {
			break;
		}

		// Process the line into result
		process_line(line, lang_freq_map, hashtag_freq_map);

		// Increment current by line length and 1 for \n
		current += line_length + 1;
	}
	is.close();
}
