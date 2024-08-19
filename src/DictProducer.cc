#include "../include/DictProducer.h"
#include "../include/SplitTool.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype> //字符相关，例如tolower库
#include <algorithm>
#include <string>
#include <cstddef>  // 包含 size_t 的定义


using std::cout;
using std::cerr;
using std::ostringstream;
using std::istringstream;
using std::ofstream;
using std::ifstream;
using std::map;
using std::size_t;
using std::sort;
using std::ispunct;
using std::tolower;
using std::isdigit;


//查看原始数据
void DictProducer::showRaw(){
    cout << this->_raw;
}

//查看原始数据
string DictProducer::getRaw(){
    return this->_raw;
}

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

//读入所有原始数据
void DictProducer::setRaw(){
    for(auto & ele : this->_files){
        this->_raw.append(readFileToString(ele));
    }
    cout << "setRaw data: readfile to string success! \n";
}

//英文数据清洗：大小写转换，标点符号替换
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

//中文数据清洗：标点符号替换
// 检查字符是否是汉字
bool isChineseCharacter(const std::string& str, size_t pos) {
    if (pos + 2 >= str.size()) return false; // 需要至少3个字节

    unsigned char c1 = str[pos];
    unsigned char c2 = str[pos + 1];
    unsigned char c3 = str[pos + 2];

    // UTF-8 处理
    if (c1 < 0x80) return false; // ASCII范围内的字符

    // 汉字的 UTF-8 编码范围 (3字节)
    if (c1 >= 0xE4 && c1 <= 0xE9) {
        if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
            return true; // 这范围包含了常用汉字
        }
    }
    return false;
}

void DictProducer::cleanCnMaterial() {
    std::string result;
    size_t pos = 0;

    while (pos < this->_raw.size()) {
        if (isChineseCharacter(this->_raw, pos)) {
            result.push_back(this->_raw[pos]);
            result.push_back(this->_raw[pos + 1]);
            result.push_back(this->_raw[pos + 2]);
            pos += 3; // 跳过完整的汉字
        } else {
            pos++;
        }
    }

    this->_raw = result;
    std::cout << "cleanCnMaterial Success!\n";
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


// 构建中文词典
void DictProducer::buildCnDict(){
    map<string, int> wordCount;
    vector<string>result = _cuttor->cut(_raw);
    for(auto &word : result){
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
    std::cout << "buildCnDict Success!\n";
}

// 存储推荐词
void DictProducer::storeDict(const string& filename) {
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


void DictProducer::createIndex(const string& filename) {
    // 打开文件
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << filename << "\n";
        return;
    }

    string line;
    int lineIndex = 0; // 记录行索引

    // 逐行读取文件内容
    while (getline(file, line)) {
        // 使用字符串流提取单词
        istringstream iss(line);
        string word;

        // 逐个单词读取并存储到索引中
        while (iss >> word) {
            // 检查单词是英文单词还是中文词语
            if (isalpha(word[0])) {  // 如果第一个字符是字母，处理为英文单词
                for (char c : word) {
                    _Index[string(1, c)].insert(lineIndex); // 拆分成字母并存入索引
                }
            } else {  // 处理为中文词语
                for (size_t i = 0; i < word.size(); ++i) {
                    // 处理 UTF-8 编码的汉字
                    // 获取字符的字节长度
                    size_t charLength = 0;
                    if ((word[i] & 0x80) == 0) {
                        charLength = 1; // ASCII字符
                    } else if ((word[i] & 0xE0) == 0xC0) {
                        charLength = 2; // 中文字符（2字节）
                    } else if ((word[i] & 0xF0) == 0xE0) {
                        charLength = 3; // 中文字符（3字节）
                    } else if ((word[i] & 0xF8) == 0xF0) {
                        charLength = 4; // 中文字符（4字节）
                    } else {
                        // 无效字符，跳过
                        charLength = 1; // 默认处理单字节
                    }

                    // 提取汉字并插入索引
                    string character = word.substr(i, charLength);
                    _Index[character].insert(lineIndex);
                    i += charLength - 1; // 跳过当前字符的字节数
                }
            }
        }
        lineIndex++; // 行索引递增
    }
}

// 存储索引词典
void DictProducer::storeIndexDict(const string& filename){
    // 使用 POSIX 的 open 函数打开文件（如果文件存在则追加）
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        cerr << "Error opening file: " << filename << "\n";
        perror("Detailed error");
        return;
    }

    // 遍历 _Index 并将内容写入文件
    for (const auto& entry : _Index) {
        const string& word = entry.first; // 单词或汉字
        const set<int>& indices = entry.second; // 行索引集合

        // 构建输出字符串
        ostringstream oss;
        oss << word; // 添加单词或汉字

        // 添加索引到字符串
        for (int index : indices) {
            oss << " " << index; // 用空格分隔
        }

        oss << "\n"; // 换行

        // 将字符串写入文件
        string output = oss.str();
        write(fd, output.c_str(), output.size()); // 写入字符串到文件
    }

    // 关闭文件
    close(fd);
}