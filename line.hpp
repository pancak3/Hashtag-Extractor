#include <unordered_map>

using std::string;
using std::unordered_map;

/**
 * Retrieve hashtags from line, and calculate frequencies of them
 */
void process_line(const string& line,
				  unordered_map<string, unsigned long>& lang_freq_map,
				  unordered_map<string, unsigned long>& hashtag_freq_map);
