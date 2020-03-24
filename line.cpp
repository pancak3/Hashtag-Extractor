// References:
// https://stackoverflow.com/questions/17337602/how-to-get-error-message-when-ifstream-open-fails
#include <iostream>
#include <regex>
#include <unordered_map>
#include "include/rapidjson/document.h"

using namespace std;
using namespace rapidjson;

/**
 * Lower case a string
 */
string to_lower(string in);

/**
 * Pattern used to match hashtags
 */
regex pattern_hashtag("#[\\d\\w]+");

/**
 * Retrieve hashtags from line, and calculate frequencies of them
 * @param line string of a line (string), e.g.: "This is a tweet!"
 * @param lang_freq_map frequency map of languages (unordered_map), e.g.:
 * lang_freq_map["en"] -> 42
 * @param hashtag_freq_map frequency map of hashtags (unordered_map), e.g.:
 * hashtag_freq_map["#hashtag"] -> 43
 */
void process_line(const string& line,
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

/**
 * Lower case a string
 * @param in an input string (string), e.g.: "#ANice_Day"
 * @return the lower case of input string (string), e.g.: "#anice_day"
 */
string to_lower(string in) {
	for (char& i : in)
		if ('A' <= i && i <= 'Z')
			i += 32;
	return in;
}
