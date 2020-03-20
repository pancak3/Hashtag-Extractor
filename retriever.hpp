#include <fstream>
#include <map>

using namespace std;

/***
 * the res structure
 *  res.hash_tag_freq_map stores the hash_tags and frequencies
 *  res.lang stores the language
 *  res.is_empty is true if this res is empty
 */
struct ResRetriever {
    map<string, int> hash_tag_freq_map;
    string lang;
    bool is_empty = {true};
};
/**
 *
 * @param file
 * @return res in ResRetriever
 */

ResRetriever infoRetriever(ifstream &file);

/**
 *
 * @param in : input string
 * @return lowercase of input string, popped the last character
 */
string to_lower(string in);

/**
 *  demo func to use infoRetriever()
 */
void demo();