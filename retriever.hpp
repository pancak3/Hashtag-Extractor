#include <unordered_map>

using std::string;
using std::unordered_map;

/**
 *
 */
void process_line(string line,
				  unordered_map<string, unsigned long>& lang_freq_map,
				  unordered_map<string, unsigned long>& hash_tag_freq_map);
