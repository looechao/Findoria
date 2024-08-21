#include <iostream>
#include <fstream>

//this define can avoid some logs which you don't need to care about.
#define LOGGER_LEVEL LL_WARN 

#include "../include/simhash/Simhasher.hpp"
using namespace simhash;


const char* const DICT_PATH = "../include/cppjieba/dict/jieba.dict.utf8";
const char* const HMM_PATH = "../include/cppjieba/dict/hmm_model.utf8";
const char* const IDF_PATH = "../include/cppjieba/dict/idf.utf8";
const char* const STOP_WORD_PATH = "../include/cppjieba/dict/stop_words.utf8";

int main(int argc, char** argv)
{
    Simhasher simhasher(DICT_PATH, HMM_PATH, IDF_PATH, STOP_WORD_PATH);
    string s("我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，走上人生巅峰。");
    size_t topN = 5;
    uint64_t u64 = 0;
    vector<pair<string ,double> > res;
    //提取关键词到res
    simhasher.extract(s, res, topN);
    //计算simhash数值
    simhasher.make(s, topN, u64);

    string s1 = "清晨的阳光透过树梢，洒在地面上，斑驳的光影仿佛一幅静谧的画卷。微风轻拂，带来了淡淡的花香，让人不禁深吸一口气，感受到大自然的宁静与美好。";
    string s2 = "初晨的阳光穿过树叶，洒在地上，斑驳的光影仿佛一幅静默的画作。微风轻吹，夹带着淡淡的花香，让人情不自禁地深吸一口气，感受到大自然的静谧与美丽。";
    uint64_t u1, u2;
    simhasher.make(s1, 5, u1);
    simhasher.make(s2, 5, u2);
    vector<pair<string ,double> > res1;
    vector<pair<string ,double> > res2;
    simhasher.extract(s1, res1, 5);
    simhasher.extract(s2, res2, 5);
    
    cout<< s1 << "\n和\n" << s2 << "\n simhash值的相等判断如下：\n"<<endl;
    cout<< "海明距离阈值默认设置为3，则isEqual结果为：" << (Simhasher::isEqual(u1, u2)) << endl; 
    cout<< "海明距离阈值默认设置为5，则isEqual结果为：" << (Simhasher::isEqual(u1, u2, 5)) << endl; 
    cout << u1 << "\n";
    cout << u2 << "\n";
    return EXIT_SUCCESS;
}
