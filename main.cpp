// Entry point of program
// MPI divides work here

// References:
//
// https://stackoverflow.com/questions/3072795
// Note that comments/code may be adapted from man pages

#include <fstream>
#include <iostream>
#include <math.h>
#include <mpi.h>
#include <sys/stat.h>

long long get_file_length(char* filename);
void perform_work(char* filename, long long file_length);

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
	std::cerr << "File size (in bytes): " << file_length << std::endl;

	perform_work(argv[1], file_length);

	// Terminates MPI execution environment
	MPI::Finalize();

	return 0;
}

// Determine work down by node and joins results
void perform_work(char* filename, long long file_length) {
	int rank, size;

	// Get rank of current communicator + size
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Let's start off with even divisions and improve it if we have time
	// Note: start and end are inclusive
	long long chunk = file_length / size + (file_length % size == 0 ? 0 : 1);
	long long start = rank * chunk;
	long long end = std::min(file_length, (rank + 1) * chunk - 1);

	std::cerr << "rank " << rank << ", start: " << start << ", end: " << end
			  << std::endl;
}

// Gets length of file
long long get_file_length(char* filename) {
	struct stat sb;
	if (lstat(filename, &sb) == -1) {
		perror("lstat");
		std::exit(EXIT_FAILURE);
	}
	return sb.st_size;
}
