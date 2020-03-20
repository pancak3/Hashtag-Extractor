#include <iostream>
#include <map>
#include <string>
#include <regex>
#include "include/rapidjson/document.h"
#include "retriever.h"
#include <list>

using namespace std;
using namespace rapidjson;


regex pattern_hashTag("(?:\\s|^)#[A-Za-z0-9\\-\\.\\_]+(?:\\s|$)");


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


    // remove ,\n at the end of the line
    line.resize(line.size() - 2);

    Document d;
    d.Parse(line.c_str());
#ifdef DEBUG
    cout << "[*] Tweet content: " << d["doc"]["text"].GetString() << endl;
#endif
    regex_search(line, matched_strings, pattern_hashTag);

    // retrieve hash tags
    int counter = 0;
    for (auto x : matched_strings) {
        if (x.length()) {
            // whether contains key
            if (result.hash_tag_freq_map.end() != result.hash_tag_freq_map.find(x)) {
                result.hash_tag_freq_map[x] += 1;
            } else {
                result.hash_tag_freq_map[x] = 1;
            }
#ifdef DEBUG
            cout << x << endl;
#endif
            counter++;
        }

    }
    result.lang = d["doc"]["lang"].GetString();
#ifdef DEBUG
    cout << "[*] Language: " << d["doc"]["lang"].GetString() << endl;
    cout << endl;
#endif
    result.is_valid = true;

    return result;
};

void demo() {
    ifstream json_file;
    json_file.open("./tinyTwitter.json");
#ifdef DEBUG
    //  remove one more line for debug start the head line of the file
    string line;
    getline(json_file, line);
    cout << "[*] Line: " << line << endl;
#endif
    list<ResRetriever> res_list;
    ResRetriever res;
    if (json_file.is_open()) {
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
    for (i = res_list.begin(); i != res_list.end(); i++) {
        res = *i;
        if (!res.is_valid) continue;
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