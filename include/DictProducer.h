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

class DictProducer{
public:
    // 获取单例实例的静态方法
    static DictProducer& getInstance(const vector<string>& inputFiles, SplitTool* tool) {
        static DictProducer instance(inputFiles, tool);  // 静态局部变量，保证只初始化一次
        return instance;
    }
    // 删除拷贝构造函数和赋值运算符，防止拷贝
    DictProducer(const DictProducer&) = delete;
    DictProducer& operator=(const DictProducer&) = delete;

    ~DictProducer() {}

    void setRaw();
    void showRaw();
    void cleanEnMaterial();
    void cleanCnMaterial();
    void buildEnDict();
    void buildCnDict();
    void createIndex();
    void store(const string& filename);

private:
    // 私有构造函数，防止外部实例化
    DictProducer(const vector<string>& inputFiles, SplitTool* tool)
        : _files(inputFiles), _cuttor(tool) {}
    
    string _raw;
    vector<string> _files;
    vector<pair<string, int>> _dict;
    map<string, set<int>> _index;
    SplitTool* _cuttor;
};


#endif // __DictProducer_H__
       //
