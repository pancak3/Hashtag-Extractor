#include <fstream>
#include <map>

using namespace std;
struct ResRetriever {
    map<string, int> hash_tag_freq_map;
    string lang;
    bool is_valid = {false};
};

ResRetriever infoRetriever(ifstream &file, int line_start);

void demo();