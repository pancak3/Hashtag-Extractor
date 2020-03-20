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

#ifdef DEBUG
    //  remove one more line for debug start the head line of the file
    getline(file, line);
    cout << "[*] Line: " << line << endl;
#endif
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
    std::ifstream json_file;
    json_file.open("./tinyTwitter.json");

    list<ResRetriever> res_list;
    ResRetriever res;
    if (json_file.is_open()) {
        while (json_file) {
            res = infoRetriever(json_file);
            res_list.push_back(res);
        }
    }
    json_file.close();

    list<ResRetriever>::iterator i;
    map<std::string, int>::iterator j;
    while (i != res_list.end()) {
//        res = res_list.pop_front();
    }

}