std::pair<std::unordered_map<std::string, int>,
		  std::unordered_map<std::string, int>>
process_section(const char* filename, long long start, long long end);

void send_combined_data(std::unordered_map<std::string, int> combined_res);
