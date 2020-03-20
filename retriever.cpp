#include <iostream>
#include <map>
#include <string>
#include <regex>
#include "include/rapidjson/document.h"
#include "retriever.hpp"
#include <list>

using namespace std;
using namespace rapidjson;

//punctuations refer to https://www.regular-expressions.info/posixbrackets.html
regex pattern_hashTag("(?:\\s|^)#[A-Za-z0-9]+(?:\\s|$|[!\"\\#$%&'()*+,./:;<=>?@\\[\\\\\\]^_‘{|}~])");

string to_lower(string in) {

    for (int i = 0; i < in.length(); i++)
        if ('A' <= in[i] && in[i] <= 'Z') in[i] += 32;
    in.resize(in.length() - 1);
    return in;
}

ResRetriever infoRetriever(ifstream &file) {
    ResRetriever result;

    string line;
    Document document;
    smatch matched_strings;

    getline(file, line);

    // return when occurs empty line -> result.is_valid is false now
    if (!line.length()) {
        return result;
    }

    result.line_len = line.length();

    // remove ,\n at the end of the line
    line.resize(line.size() - 2);

    Document d;
    d.Parse(line.c_str());
#ifdef DEBUG
    cout << "[*] Tweet content: " << d["doc"]["text"].GetString() << endl;
#endif
    regex_search(line, matched_strings, pattern_hashTag);

    // retrieve hash tags
    for (auto x : matched_strings) {

        if (x.length()) {
            string x_lower = to_lower(x);
            // whether contains key
            if (result.hash_tag_freq_map.end() != result.hash_tag_freq_map.find(x_lower)) {
                result.hash_tag_freq_map[x_lower] += 1;
            } else {
                result.hash_tag_freq_map[x_lower] = 1;
            }
#ifdef DEBUG
            cout << x_lower << endl;
#endif
        }

    }
    result.lang = d["doc"]["lang"].GetString();
#ifdef DEBUG
    cout << "[*] Language: " << d["doc"]["lang"].GetString() << endl;
    cout << endl;
#endif
    result.no_valid_info = false;

    return result;
};

void demo() {
    ifstream json_file;
    json_file.open("./tinyTwitter.json");

    //  remove one more line for debug start the head line of the file
    string line;
    getline(json_file, line);
#ifdef DEBUG
    cout << "[*] Line: " << line << endl;
#endif
    list<ResRetriever> res_list;
    ResRetriever res;


    if (json_file.is_open()) {
        //loop to retrieve info from lines
        while (json_file) {
            res = infoRetriever(json_file);
            res_list.push_back(res);
        }
    }
    json_file.close();

    map<string, int> final_lang;
    map<string, int> final_hash_tag;
    list<ResRetriever>::iterator i;
    map<string, int>::iterator j;

    // count hash tag frequencies and language frequencies
    for (i = res_list.begin(); i != res_list.end(); i++) {
        res = *i;
        if (res.no_valid_info) continue;
        for (j = res.hash_tag_freq_map.begin(); j != res.hash_tag_freq_map.end(); j++) {
            //  cout << j->first << " : " << j->second << endl;
            if (final_hash_tag.end() != final_hash_tag.find(j->first)) {
                final_hash_tag[j->first] += j->second;
            } else {
                final_hash_tag[j->first] = 1;
            }
        }

        // cout << res.lang << endl << endl;
        if (final_lang.end() != final_lang.find(res.lang)) {
            final_lang[res.lang] += 1;
        } else {
            final_lang[res.lang] = 1;
        }
    }
    cout << "[*] HashTag Freq Results" << endl;

    for (j = final_hash_tag.begin(); j != final_hash_tag.end(); j++) {
        cout << j->first << " : " << j->second << endl;
    }
    cout << "[*] Language Freq Results" << endl;

    for (j = final_lang.begin(); j != final_lang.end(); j++) {
        cout << j->first << " : " << j->second << endl;
    }
}