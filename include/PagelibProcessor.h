#ifndef __PagelibProcessor_H__
#define __PagelibProcessor_H__

#define LOGGER_LEVEL LL_WARN 

#include "tinyxml2.h"
#include "../include/simhash/Simhasher.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>

using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::unordered_map;
using std::map;

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
    void createOffsetlib(); //构建页面偏移库
    void buildInvertIndexMap(); //构建倒排索引库
    void storeOffset(const string& filename);
    void storeWebIndex(const string& filename);
private:
    string _directory;
    vector<uint64_t> _simhasheLib;  //simhash
    unordered_map<int, pair<int, int>> _offset;  //页面偏移库
    unordered_map<int, pair<int, int>> _invertIndex;  //倒排索引库
};

#endif // __PagelibProcessor_H__
       //
