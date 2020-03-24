// References:
// https://stackoverflow.com/questions/17337602/how-to-get-error-message-when-ifstream-open-fails
#include <iostream>
#include <regex>
#include <unordered_map>

#include "include/rapidjson/document.h"

using namespace std;
using namespace rapidjson;

// prototype
string to_lower(string in);

// punctuations refer to
// https://www.regular-expressions.info/posixbrackets.html
regex pattern_hashtag(
	"#[A-Za-z0-9_]+(?:\\s|$|[!\"\\#$%&'()*+,./:;<-=>?@\\[\\\\\\]^‘{|}~])");

void process_line(string line,
				  unordered_map<string, unsigned long>& lang_freq_map,
				  unordered_map<string, unsigned long>& hashtag_freq_map) {
	// parse into json
	Document d;
	d.Parse(line.c_str());

	// retrieve hash tags
	smatch matched_strings;
	regex_search(line, matched_strings, pattern_hashtag);
	for (auto matched : matched_strings) {
		if (matched.length()) {
			string matched_lower = to_lower(matched);

			// whether contains key
			if (hashtag_freq_map.end() !=
				hashtag_freq_map.find(matched_lower)) {
				hashtag_freq_map[matched_lower] += 1;
			} else {
				hashtag_freq_map[matched_lower] = 1;
			}
		}
	}

	string lang = d["doc"]["lang"].GetString();
	if (lang_freq_map.find(lang) != lang_freq_map.end()) {
		lang_freq_map[lang] += 1;
	} else {
		lang_freq_map[lang] = 1;
	}
};

string to_lower(string in) {
	for (int i = 0; i < in.length(); i++)
		if ('A' <= in[i] && in[i] <= 'Z')
			in[i] += 32;
	in.pop_back();
	return in;
}
