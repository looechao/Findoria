#ifndef __PagelibProcessor_H__
#define __PagelibProcessor_H__

#include "tinyxml2.h"

#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

using std::string;
using std::vector;
using std::cout;

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
    PagelibProcessor(string path, vector<Item> result)
    :_directory(path)
    ,_cleandata(result)
    {}

    PagelibProcessor() {}
    ~PagelibProcessor() {}
    
    void readData();
    void cleanData();
    void cutRedundantPage();  //起到parse的作用，并且清除多余的内容
    void buildInvertIndexMap();
    void storeOnDisk(const string& filename);
private:
    string _directory;
    vector<Item> _cleandata;
};

#endif // __PagelibProcessor_H__
       //
