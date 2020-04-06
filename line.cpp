// Processes and extracts information from individual lines

#include <iostream>
#include <regex>
#include <unordered_map>
#include "include/rapidjson/document.h"

using namespace std;
using namespace rapidjson;

// Function prototype
string to_lower(string in);

// Pattern used to match hashtags
string pattern = "#[\\d\\w]+";
regex pattern_hashtag(pattern);

/**
 * Extract language and hashtags from line, and calculate frequencies.
 * @param line line (string), e.g.: "This is a tweet!"
 * @param lang_freq_map frequency map of languages (unordered_map), e.g.:
 * lang_freq_map["en"] -> 42
 * @param hashtag_freq_map frequency map of hashtags (unordered_map), e.g.:
 * hashtag_freq_map["#hashtag"] -> 43
 */
void process_line(const string& line,
				  unordered_map<string, unsigned long>& lang_freq_map,
				  unordered_map<string, unsigned long>& hashtag_freq_map) {
	try {
		// Parse into json
		Document d;
		d.Parse(line.c_str());

		// Avoid to extract usl params as hash tags
		string content;
		content = d["doc"]["text"].GetString();
		content += d["doc"]["user"]["description"].GetString();

		// Extract hash tags
		smatch matched_strings;

		regex_search(content, matched_strings, pattern_hashtag);
		for (auto matched : matched_strings) {
			if (matched.length()) {
				string matched_lower = to_lower(matched);

				// Whether contains key
				if (hashtag_freq_map.end() !=
					hashtag_freq_map.find(matched_lower)) {
					hashtag_freq_map[matched_lower] += 1;
				} else {
					hashtag_freq_map[matched_lower] = 1;
				}
			}
		}

		// Extract language
		string lang = d["doc"]["lang"].GetString();
		if (lang_freq_map.find(lang) != lang_freq_map.end()) {
			lang_freq_map[lang] += 1;
		} else {
			lang_freq_map[lang] = 1;
		}
	} catch (const std::regex_error& e) {
		std::cout << "regex_error caught: " << e.what() << std::endl;
		if (e.code() == std::regex_constants::error_brack) {
			std::cout << "The code was error_brack" << std::endl;
		}
	}
};

/**
 * Transform string to lowercase.
 * @param in input string (string), e.g.: "#ANice_Day"
 * @return lower case equivalent of input string (string), e.g.: "#anice_day"
 */
string to_lower(string in) {
	for (char& i : in)
		if ('A' <= i && i <= 'Z')
			i += 32;
	return in;
}
