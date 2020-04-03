#include <unordered_map>
#include <utility>

/*
 * Further subdivides the section [start, end], assigns them to threads and
 * combines results.
 */
std::pair<std::unordered_map<std::string, unsigned long>,
		  std::unordered_map<std::string, unsigned long>>
process_section(const char* filename, long long start, long long end);
