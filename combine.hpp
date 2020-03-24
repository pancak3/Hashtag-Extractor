#include <string>
#include <unordered_map>

using std::pair;
using std::string;
using std::unordered_map;

/**
 * Calls on functions to combines results from multiple nodes together and
 * print them.
 */
void combine_results(const pair<unordered_map<string, unsigned long>,
								unordered_map<string, unsigned long>>& results,
					 int rank, int size,
					 const unordered_map<string, string>& lang_map);
