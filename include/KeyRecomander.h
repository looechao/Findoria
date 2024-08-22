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
using std::size_t;

class CandidateResult{
public:
    // 可以添加构造函数以便于初始化
    CandidateResult(const string& word = "", int freq = 0, int dist = 0)
        : _word(word), _freq(freq), _dist(dist) {}

    string _word;
    int _freq;
    int _dist;
};

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

    ~KeyRecomander() {}

    // 词典
    vector<pair<string, int>> _Dict;
    map<string, set<int>> _Index;
    
    //结果
    vector<CandidateResult> _recomandWord;

    void loadDict(const string& filename);
    void loadIndex(const string& filename);

    //生成候选词
    vector<CandidateResult> generateCandidates(vector<string> splited_words, string query);
    vector<CandidateResult> sortCandidates(vector<CandidateResult> candidates);  //用最小编辑距离进行排序
    vector<string> split_query(string query);
    
    

private:
    // 私有构造函数
    KeyRecomander() {
        // 初始化代码
    }
};

#endif // __KeyRecomander_H__
       //
