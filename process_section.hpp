void process_section(const char *filename, long long start, long long end,int rank, int num_worker);

void send_combined_data(std::unordered_map<std::string, int> combined_res);
