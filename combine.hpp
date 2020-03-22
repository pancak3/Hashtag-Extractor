#include <string>
#include <unordered_map>

using std::pair;
using std::string;
using std::unordered_map;

void combine_results(pair<unordered_map<string, unsigned long>,
						  unordered_map<string, unsigned long>>
						 results,
					 int rank, int size,
					 unordered_map<string, string> country_codes);