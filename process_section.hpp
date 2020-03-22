#include <unordered_map>
#include <utility>

std::pair<std::unordered_map<std::string, unsigned long>,
		  std::unordered_map<std::string, unsigned long>>
process_section(const char* filename, long long start, long long end);

void send_combined_data(
	std::unordered_map<std::string, unsigned long> combined_res);
