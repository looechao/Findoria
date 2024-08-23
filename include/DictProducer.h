#ifndef __DictProducer_H__
#define __DictProducer_H__

#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <utility>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "SplitTool.h"

using std::pair;
using std::set;
using std::map;
using std::string;
using std::vector;

class DictProducer {
public:
    // 公有构造函数，允许外部实例化
    DictProducer(const vector<string>& inputFiles, SplitTool* tool)
        : _files(inputFiles), _cuttor(tool) {}

    // 删除拷贝构造函数和赋值运算符，防止拷贝
    DictProducer(const DictProducer&) = delete;
    DictProducer& operator=(const DictProducer&) = delete;

    ~DictProducer() {}

    void setRaw();
    string getRaw();
    void showRaw();
    void cleanEnMaterial();
    void cleanCnMaterial();
    void buildEnDict();
    void buildCnDict();
    void createIndex(const string& filename);
    void storeDict(const string& filename);
    void storeIndexDict(const string& filename);

private:
    string _raw;
    vector<string> _files;
    vector<pair<string, int>> _Dict; 
    map<string, set<int>> _Index;  //通过字母和汉字找到词典中对应的包含它的词语
    SplitTool* _cuttor;
};

#endif // __DictProducer_H__