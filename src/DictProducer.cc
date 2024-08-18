#include "../include/DictProducer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype> //字符相关，例如tolower库
#include <algorithm>
#include <string>

using std::cout;
using std::cerr;
using std::ostringstream;
using std::istringstream;
using std::ofstream;
using std::sort;
using std::ispunct;
using std::tolower;
using std::isdigit;

//读取路径下的所有文件中的内容
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

//查看原始数据
void DictProducer::showRaw(){
    cout << this->_raw;
}

//读入所有原始数据
void DictProducer::setRaw(){
    cout << "setRaw data:\n";
    for(auto & ele : this->_files){
        this->_raw.append(readFileToString(ele));
    }
    cout << "readfile to string success! \n";
}

//数据清洗：大小写转换，标点符号替换
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

//构建词典
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

// 存储推荐词
void DictProducer::store(const std::string& filename) {
    // 使用 POSIX 的 open 函数打开文件（如果文件存在则追加）
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        std::cerr << "Error opening file: " << filename << "\n";
        perror("Detailed error");
        return;
    }

    // 写入字典内容
    for (const auto& pair : _dict) {
        // 假设 pair.first 是字符串，pair.second 是 int 类型
        string line = pair.first + " " + std::to_string(pair.second) + "\n";

        // 将构建的行写入文件
        ssize_t bytesWritten = write(fd, line.c_str(), line.size());
        if (bytesWritten < 0) {
            std::cerr << "Error writing to file: " << filename << "\n";
            perror("Detailed error");
            break; // 发生错误时退出循环
        }
    }

    // 关闭文件描述符
    close(fd);
    std::cout << "Data written to file: " << filename << "\n";
}