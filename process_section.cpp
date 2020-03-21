// Work for one process is divided into threads
// Division into threads, file reading and joining are managed here

// References:
// https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
// http://www.cplusplus.com/reference/fstream/ifstream/ifstream/
// http://www.cplusplus.com/reference/thread/thread/
// https://thispointer.com/c11-how-to-create-vector-of-thread-objects/
// https://stackoverflow.com/questions/823479

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <thread>
#include <vector>
#include "retriever.hpp"

void process_section_thread(char* filename, long long start, long long end);

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

	// Vec for thread
	std::vector<std::thread> threads;

	// Again, split task into chunks
	long long total = (end - start) + 1;
	long long chunk = total / n + (total % n == 0 ? 0 : 1);
	for (int i = 0; i < n; i++) {
		long long thread_start = i * chunk + start;
		long long thread_end = std::min(total, (i + 1) * chunk - 1) + start;
		if (i == n - 1) {
			thread_end = end;
		}

		// Create threads
		threads.push_back(std::thread(process_section_thread, filename,
									  thread_start, thread_end));
	}

	// Finish up
	for (std::thread& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

// Actually process the section [start, end]
void process_section_thread(char* filename, long long start, long long end) {
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
	map<string, int> lang_freq_map;
	map<string, int> hashtag_freq_map;
	long line_length;

	while (is.good() && current <= end) {
		// Read line
		getline(is, line);
		// Bad line?
		if (line.length() == 0) {
			continue;
		}

		// Process the line into result
		process_line(line, lang_freq_map, hashtag_freq_map);

		// Does this include \r\n?
		current += line.length();
	}
	is.close();

#ifdef DEBUG
	map<string, int>::iterator j;
	cout << "[*] HashTag Freq Results" << endl;
	for (j = hashtag_freq_map.begin(); j != hashtag_freq_map.end(); j++) {
		cout << j->first << " : " << j->second << endl;
	}

	cout << "[*] Language Freq Results" << endl;
	for (j = lang_freq_map.begin(); j != lang_freq_map.end(); j++) {
		cout << j->first << " : " << j->second << endl;
	}
#endif
}
