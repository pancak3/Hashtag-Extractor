// Entry point of program
// MPI divides work here

// References:
//
// https://stackoverflow.com/questions/3072795
// Note that comments/code may be adapted from man pages

#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sys/stat.h>

long long get_file_length(char* filename);

int main(int argc, char** argv) {
	// Init executation environment
	MPI::Init(argc, argv);

	if (argc < 3) {
		std::cerr << "usage: " << argv[0] << " "
				  << "input.json country_codes.csv" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Get number of bytes in file
	unsigned long long file_length = get_file_length(argv[1]);
	std::cout << "File size (in bytes): " << file_length << std::endl;

	// Terminates MPI execution environment
	MPI::Finalize();

	return 0;
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
