#include <string>
#include <unordered_map>

using std::pair;
using std::string;
using std::unordered_map;

void combine(pair<unordered_map<string, unsigned long>,
				  unordered_map<string, unsigned long>>
				 results,
			 int rank, int size);