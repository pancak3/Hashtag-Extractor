#include "retriever.hpp"
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <string>
#include "include/rapidjson/document.h"

using namespace std;
using namespace rapidjson;

// prototype
string to_lower(string in);

// punctuations refer to
// https://www.regular-expressions.info/posixbrackets.html
regex pattern_hashtag("(?:\\s|^)#[A-Za-z0-9]+(?:\\s|$|[!\"\\#$%&'()*+,./"
					  ":;<=>?@\\[\\\\\\]^_‘{|}~])");

void process_line(string line, map<string, int>& lang_freq_map,
				  map<string, int>& hashtag_freq_map) {
	// remove ",\r" at the end of the line
	// TODO: not always true...
	line.resize(line.size() - 2);

	// parse into json
	Document d;
	d.Parse(line.c_str());
#ifdef DEBUG
	cout << "[*] Tweet content: " << d["doc"]["text"].GetString() << endl;
#endif

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
#ifdef DEBUG
			cout << matched_lower << endl;
#endif
		}
	}

	string lang = d["doc"]["lang"].GetString();
	if (lang_freq_map.find(lang) != lang_freq_map.end()) {
		lang_freq_map[lang] += 1;
	} else {
		lang_freq_map[lang] = 1;
	}

#ifdef DEBUG
	cout << "[*] Language: " << d["doc"]["lang"].GetString() << endl;
	cout << endl;
#endif
};

string to_lower(string in) {
	for (int i = 0; i < in.length(); i++)
		if ('A' <= in[i] && in[i] <= 'Z')
			in[i] += 32;
	in.resize(in.length() - 1);
	return in;
}
