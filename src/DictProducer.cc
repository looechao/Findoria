#include "../include/DictProducer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype> //字符相关，例如tolower库
#include <algorithm>

using std::cout;
using std::cerr;
using std::ostringstream;
using std::istringstream;
using std::ofstream;
using std::sort;
using std::ispunct;
using std::tolower;
using std::isdigit;

string readFileToString(string filePath){
    std::ifstream file(filePath);
    if(!file.is_open()){
        cerr << "Error opening file: " << filePath << "\n"; 
        return "";
    }

    ostringstream oss;
    oss << file.rdbuf();
    file.close();
    return oss.str();
}

void DictProducer::showRaw(){
    cout << this->_raw;
}

void DictProducer::setRaw(){
    cout << "setRaw data:\n";
    for(auto & ele : this->_files){
        this->_raw.append(readFileToString(ele));
    }
    cout << "readfile to string success! \n";
}

void DictProducer::cleanEnMaterial(){
    for(auto &ch: this->_raw){
        if(ispunct(ch) || isdigit(ch)){
            ch = ' ';
        }else{
            ch = tolower(ch);
        }
    }
    cout << "celanEnMaterial Success!\n";
}

void DictProducer::buildEnDict(){
    map<string, int> wordCount;
    istringstream iss(this->_raw);
    string word;

    // 从字符串中读取单词并统计词频
    while (iss >> word) {
        // 去除单词两端的标点符号
        while (!word.empty() && std::ispunct(word.back())) {
            word.pop_back();
        }
        while (!word.empty() && std::ispunct(word.front())) {
            word.erase(word.begin());
        }

        // 将单词转换为小写
        for (char& ch : word) {
            ch = std::tolower(ch);
        }

        // 统计词频
        ++wordCount[word];
    }

    // 将统计结果转换为 vector<pair<string, int>> 格式
    for (const auto& pair : wordCount) {
        _dict.push_back(pair);
    }

    sort(_dict.begin(), _dict.end(), [](const pair<string, int>& a, const pair<string, int>& b){
        return a.second > b.second;
    });

    std::cout << "buildEnDict Success!\n";
}