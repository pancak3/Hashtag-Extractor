#include <string>
#include <unordered_map>

using std::pair;
using std::string;
using std::unordered_map;

/**
 * Calls on functions to combine results from multiple processes together and
 * print them.
 */
void combine_results(const pair<unordered_map<string, unsigned long>,
								unordered_map<string, unsigned long>>& results,
					 int rank, int size,
					 const unordered_map<string, string>& lang_map);

/**
 * Send maps (results) to the destination MPI process.
 */
void send_results(int dest, unordered_map<string, unsigned long>& freq_map);

/**
 * Receive maps (results) from the source MPI process.
 */
void recv_results(int source, unordered_map<string, unsigned long>& freq_map);