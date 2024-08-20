#ifndef __KeyRecomander_H__
#define __KeyRecomander_H__

#include <iostream>
#include <map>
#include <set>
#include <vector>

using std::pair;
using std::vector;
using std::map;
using std::set;
using std::string;

class KeyRecomander {
public:
    // 获取单例实例的方法
    static KeyRecomander& getInstance() {
        static KeyRecomander instance; // 懒汉式单例，第一次调用时创建实例
        return instance;
    }
    // 禁止拷贝构造和赋值操作
    KeyRecomander(const KeyRecomander&) = delete;
    KeyRecomander& operator=(const KeyRecomander&) = delete;

    // 词典
    vector<pair<string, int>> _Dict;
    map<string, set<int>> _Index;

    //生成候选词
    // vector<string> generateCandidates(vector<string> splited_words);
    // vector<string> sort(vector<string> recomandation);
    vector<string> split_query(string query);

private:
    // 私有构造函数
    KeyRecomander() {
        // 初始化代码
    }
};

#endif // __KeyRecomander_H__
       //
