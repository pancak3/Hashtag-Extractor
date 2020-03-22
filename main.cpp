// Entry point of program
// MPI divides work here

// References:
// https://stackoverflow.com/questions/14718124
// https://stackoverflow.com/questions/5122804
// https://stackoverflow.com/questions/31323135
// https://stackoverflow.com/questions/5689003
// Note that comments/code may be adapted from man pages

#include <fstream>
#include <mpi.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include "main_combine.hpp"
#include "process_section.hpp"

using std::pair;
using std::string;
using std::unordered_map;

// Prototypes
long long get_file_length(const char* filename);
void perform_work(const char* filename, long long file_length,
				  unordered_map<string, string>& country_codes);
unordered_map<string, string> read_country_csv(const char* filename);

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
