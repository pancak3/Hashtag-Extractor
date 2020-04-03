// Entry point of program
// Initial scan of input and MPI work division occurs here

// References:
// man pages for various library functions
// Language codes taken from:
// https://developer.twitter.com/en/docs/tweets/rules-and-filtering/overview/premium-operators
// https://stackoverflow.com/questions/14718124

#include <chrono>
#include <ctime>
#include <fstream>
#include <mpi.h>
#include <omp.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include "combine.hpp"
#include "threading.hpp"

using std::pair;
using std::string;
using std::unordered_map;

// Function prototypes
long long get_file_length(const char* filename);

void perform_work(const char* filename, long long file_length,
				  unordered_map<string, string>& lang_map);

unordered_map<string, string> read_lang_csv(const char* filename);

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "usage: " << argv[0] << " "
				  << "input.json lang_codes.csv" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	auto start_ts = std::chrono::system_clock::now();

	// Init execution environment
	MPI::Init(argc, argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Get number of bytes in file
	long long file_length = get_file_length(argv[1]);

	// Read country code CSV
	// Assuming that there's not much overhead in reading a small file...
	std::unordered_map<string, string> lang_map = read_lang_csv(argv[2]);

	// Split using MPI, perform work and print out results
	perform_work(argv[1], file_length, lang_map);

	// Terminate MPI execution environment
	MPI::Finalize();

	// Time taken
	if (rank == 0) {
		std::chrono::duration<double> elapsed_seconds =
			std::chrono::system_clock::now() - start_ts;
		std::cout << std::endl
				  << "[*] Time cost: " << elapsed_seconds.count() << " seconds"
				  << std::endl;
	}

	return 0;
}

/**
 * Splits and assigns work to each MPI process, joins and prints results.
 * @param filename path of twitter file
 * @param file_length length of twitter file in bytes
 * @param lang_map map of <identifier, language> pairs
 */
void perform_work(const char* filename, const long long file_length,
				  unordered_map<string, string>& lang_map) {
	// Get rank and size of current communicator
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Print file size
	if (rank == 0) {
		std::stringstream m;
		m << "[*] File " << filename << " (in bytes): "
		  << " " << file_length << std::endl;
		std::cerr << m.str();
	}

	// Divide file into chunks by bytes
	// Each MPI process will be allocated with a chunk
	// Start and end are inclusive
	long long chunk = file_length / size + (file_length % size == 0 ? 0 : 1);
	long long start = rank * chunk;
	long long end = std::min(file_length, (rank + 1) * chunk - 1);
	if (rank == size - 1) {
		end = file_length - 1;
	}

#ifdef DEBUG
	// Print chunks allocated to processes
	std::stringstream m;
	m << "Rank " << rank << " (" << omp_get_max_threads() << " threads)"
	  << " assigned: " << start << ", " << end << std::endl;
	std::cerr << m.str();
#endif

	// For the current process, divide the work further (into threads)
	// Though it's possible to have 1 MPI process for each core, use threads
	// instead to reduce network communication overheads
	pair<unordered_map<string, unsigned long>,
		 unordered_map<string, unsigned long>>
		results = process_section(filename, start, end);

	// Combine results from multiple process and print
	combine_results(results, rank, size, lang_map);
}

/**
 * Returns length of indicated file.
 * @param filename path to file
 * @return length of file in bytes
 */
long long get_file_length(const char* filename) {
	struct stat sb {};
	if (lstat(filename, &sb) == -1) {
		perror("lstat");
		std::exit(EXIT_FAILURE);
	}
	return sb.st_size;
}

/**
 * Reads csv file of language codes into <identifier, language> pairs.
 * @param filename path of language file
 * @return map of <identifier, language> pairs
 */
unordered_map<string, string> read_lang_csv(const char* filename) {
	unordered_map<string, string> lang_map;
	std::ifstream is(filename, std::ifstream::in);
	if (is.fail()) {
		std::cerr << "Cannot access language file" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	string line;
	while (is.good()) {
		// Read line
		getline(is, line);

		// Split and insert
		size_t pos = line.rfind(',');
		lang_map[line.substr(pos + 1, line.length())] = line.substr(0, pos);
	}
	is.close();
	return lang_map;
}
