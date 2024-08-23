#ifndef __WebPageSearcher_H__
#define __WebPageSearcher_H__
#include "SplitTool.h"

#include <iostream>
#include <unordered_map>
#include <utility>
#include <set>
#include <vector>

using std::unordered_map;
using std::pair;
using std::set;
using std::string;
using std::vector;

class WebPageSearcher
{
public:
    //单例构造
    static WebPageSearcher& getInstance() {
        static WebPageSearcher instance; // 懒汉式单例，第一次调用时创建实例
        return instance;
    }
    // 禁止拷贝构造和赋值操作
    WebPageSearcher(const WebPageSearcher&) = delete;
    WebPageSearcher& operator=(const WebPageSearcher&) = delete;
    
    ~WebPageSearcher() {}

    //网页数据
    unordered_map<int, pair<int, int>> _offset;  //页面偏移库 docid, pos, length
    unordered_map<string, set<pair<int, double>>> _invertIndex;  //倒排索引库
    int testMember;

    //函数，读取文件中的数据结构
    void loadOffset(const string& filename);
    string getDocContent(const string &src, int docid, unordered_map<int, pair<int, int>> offset);

    void loadInvertIndex(const string& filename);
    vector<string> splitSentence(const string& sentence);
    set<int> queryDocuments(const string& sentence);
private:
    // 私有构造函数
    WebPageSearcher() {}
};

#endif // __WebPageSearcher_H__
       //
