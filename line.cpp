// Processes and extracts information from individual lines

#include <iostream>
#include <regex>
#include <set>
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
		// Parse into JSON DOM
		Document d;
		d.Parse(line.c_str());

		// Extract hash tags
		set<string> unique_hashtags;
		smatch matched_strings;

		// Extract hash tags from tweet text
		string tweet_text;
		tweet_text = d["doc"]["text"].GetString();
		regex_search(tweet_text, matched_strings, pattern_hashtag);
		for (auto matched : matched_strings) {
			if (matched.length()) {
				string matched_lower = to_lower(matched);
				unique_hashtags.insert(matched_lower);
			}
		}

		// Extract hash tags from doc->entities->hashtags
		const Value& hashtags = d["doc"]["entities"]["hashtags"];
		assert(hashtags.IsArray());
		for (auto& v : hashtags.GetArray()) {
			string hashtag = "#";
			hashtag.append(v["text"].GetString());
			regex_search(hashtag, matched_strings, pattern_hashtag);
			for (auto filtered : matched_strings) {
				if (filtered.length() == hashtag.length()) {
					string filtered_lower = to_lower(filtered);
					unique_hashtags.insert(filtered_lower);
				}
			}
		}

		// Count freq
		for (const auto& unique_hashtag : unique_hashtags) {
			// Whether contains key
			if (hashtag_freq_map.end() !=
				hashtag_freq_map.find(unique_hashtag)) {
				hashtag_freq_map[unique_hashtag] += 1;
			} else {
				hashtag_freq_map[unique_hashtag] = 1;
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
