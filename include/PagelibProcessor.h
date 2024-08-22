#ifndef __PagelibProcessor_H__
#define __PagelibProcessor_H__

#define LOGGER_LEVEL LL_WARN 

#include "tinyxml2.h"
#include "../include/simhash/Simhasher.hpp"
#include "../include/SplitTool.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <cmath>
#include <utility>
#include <vector>

using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::unordered_map;
using std::map;
using std::set;

class Item{
public:
    string title;
    string link;
    string description;
    string content;

    void print() const {
        cout << "Title: " << title << "\n";
        cout << "Link: " << link << "\n";
        cout << "Description: " << description << "\n";
        cout << "Content: " << content << "\n";
    }
};

class PagelibProcessor
{
public:
    PagelibProcessor(string path)
    :_directory(path)
    {}

    PagelibProcessor() {}
    ~PagelibProcessor() {}
    

    void storeRawOnDisk(const string& filename);
    void cutRedundantPage(const string& src, const string& des);  //页面去重，simhash算法  
    void createOffsetlib(const string& src); //构建页面偏移库
    void buildInvertIndexMap(); //构建倒排索引库
    void storeOffset(const string& filename);
    void storeInvertIndexMap(const string& filename);
    unordered_map<string, int> calculateTermFrequency(const vector<string> &tokens);  //计算词频

private:
    string _directory;
    vector<uint64_t> _simhasheLib;  //simhash
    unordered_map<int, pair<int, int>> _offset;  //页面偏移库 docid, pos, length
    unordered_map<string, set<pair<int, double>>> _invertIndex;  //倒排索引库
};

#endif // __PagelibProcessor_H__
       //
