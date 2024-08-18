#ifndef __DictProducer_H__
#define __DictProducer_H__

#include <map>
#include <set>
#include <iostream>
#include <vector>
#include "SplitTool.h"

using std::pair;
using std::set;
using std::map;
using std::string;
using std::vector;

class DictProducer{
public:
    DictProducer() {}
    DictProducer(vector<string> inputFiles, SplitTool* tool)
    :_files(inputFiles)
    ,_cuttor(tool)
    {}

    ~DictProducer() {}

    void cleanEnMaterial();
    void cleanCnMaterial();
    void buildEnDict();
    void buildCnDict();
    void createIndex();
    void store();

private:
    vector<string> _files;
    vector<pair<string, int>> _dict;
    map<string, set<int>> _index;
    SplitTool* _cuttor;
};


#endif // __DictProducer_H__
       //
