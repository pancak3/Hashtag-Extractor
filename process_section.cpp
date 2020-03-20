// Work for one process is divided into threads
// Division into threads, file reading and joining are managed here

// References:
// https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
// http://www.cplusplus.com/reference/thread/thread/
// https://thispointer.com/c11-how-to-create-vector-of-thread-objects/
// https://stackoverflow.com/questions/823479

#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <sys/mman.h>
#include <thread>
#include <vector>

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
	std::stringstream m;
	m << start << " " << end << std::endl;
	std::cerr << m.str();
}
