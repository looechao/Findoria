#include "../include/WebPageSearcher.h"
#include "../include/SplitTool.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

using std::cerr;
using std::cout;
using std::ifstream;
using std::ios;
using std::pair;
using std::set;
using std::size_t;
using std::string;
using std::unordered_map;
using std::vector;
using std::set_intersection;

SplitTool *cnCuttor = new SplitToolCppJieba();

string WebPageSearcher::getDocContent(const string &src, int docid, unordered_map<int, pair<int, int>> offset)
{
    ifstream infile(src, std::ios::binary);
    if (!infile.is_open())
    {
        cerr << "Unable to open file: " << src << "\n";
        return "";
    }

    if (offset.find(docid) == offset.end())
    {
        cerr << "DocID not found: " << docid << "\n";
        return "";
    }

    auto [start, length] = offset[docid];

    infile.seekg(start, std::ios::beg);
    string content(length, '\0');
    infile.read(&content[0], length);

    infile.close();
    return content;
}

void WebPageSearcher::loadOffset(const string &filename)
{
    cout << "Loading offset from file\n";
    ifstream infile(filename);
    if (!infile.is_open())
    {
        cerr << "Unable to open file for reading: " << filename << "\n";
        return;
    }

    size_t size;
    infile >> size; // 读取大小
    if (!infile)
    {
        cerr << "Error reading size!" << "\n";
        return;
    }
    cout << "size: " << size << "\n";

    if (size <= 0)
    {
        cerr << "No valid data to read!" << "\n";
        return;
    }

    // 逐个读取map的内容
    for (size_t i = 0; i < size; ++i)
    {
        int docid, start, length;
        infile >> docid >> start >> length; // 从文件中读取docid, start, length
        if (!infile)
        {
            cerr << "Error reading data at index " << i << "\n";
            return; // 读取失败，返回
        }
        _offset[docid] = {start, length}; // 保存到_offset映射中
    }

    infile.close();
    cout << "Offset loaded successfully\n";
}

void printInvertIndex(unordered_map<string, set<pair<int, double>>> &_invertIndex)
{
    cout << "Invert Index Contents:\n";
    for (const auto &entry : _invertIndex)
    {
        const string &word = entry.first;                        // 当前单词
        const set<pair<int, double>> &docWeights = entry.second; // 当前单词对应的文档ID和权重集合

        cout << "Word: " << word << "\n";
        cout << "Documents and Weights:\n";

        for (const auto &docWeight : docWeights)
        {
            int docid = docWeight.first;      // 文档ID
            double weight = docWeight.second; // 权重
            cout << "  DocID: " << docid << ", Weight: " << weight << "\n";
        }
        cout << "\n"; // 分隔不同单词的输出
    }
}

void WebPageSearcher::loadInvertIndex(const string &filename)
{
    ifstream infile(filename, ios::binary);

    if (!infile.is_open())
    {
        cerr << "Unable to open file for reading: " << filename << "\n";
    }

    // Read the size of the inverted index
    size_t size;
    infile.read(reinterpret_cast<char *>(&size), sizeof(size));

    // Read each term and its associated document ID and score pairs
    for (size_t i = 0; i < size; ++i)
    {
        // Read the length of the term
        size_t termLength;
        infile.read(reinterpret_cast<char *>(&termLength), sizeof(termLength));

        // Read the term itself
        string term(termLength, '\0'); // Create a string of the appropriate length
        infile.read(&term[0], termLength);

        // Read the number of entries for this term
        size_t entryCount;
        infile.read(reinterpret_cast<char *>(&entryCount), sizeof(entryCount));

        // Read each (document ID, score) pair
        set<pair<int, double>> docScores;
        for (size_t j = 0; j < entryCount; ++j)
        {
            int docID;
            double score;
            infile.read(reinterpret_cast<char *>(&docID), sizeof(docID));
            infile.read(reinterpret_cast<char *>(&score), sizeof(score));

            // Insert the pair into the set
            docScores.insert({docID, score});
        }

        // Insert the term and its associated set into the map
        _invertIndex[term] = docScores;
    }

    infile.close();
    cout << "invertIndex load successfully!\n";
}

vector<string> WebPageSearcher::splitSentence(const string &sentence)
{
    return cnCuttor->cut(sentence);
}

set<int> WebPageSearcher::queryDocuments(const string &sentence)
{
    set<int> result;

    vector<string> words = splitSentence(sentence);
    for(auto &it : words){
        cout << it << " ";
    }
    cout << "\n";

    // 遍历每个词
    for (const auto &word : words)
    {
        cout << word << ", 查找该词在倒排索引中的记录\n";
        // 查找该词在倒排索引中的记录
        const auto &it = _invertIndex.find(word);
        if (it != _invertIndex.end())
        {
            const auto &docSet = it->second; // 获取该词的docid集合

            // 如果是第一个词，初始化结果为该词的文档集合
            if (result.empty())
            {
                for (const auto &doc : docSet)
                {
                    result.insert(doc.first); // doc.first 是 docid
                }
                cout << "docid has been added\n";
            }
            else
            {
                // 对后续的词，进行交集操作
                set<int> currentDocs;
                for (const auto &doc : docSet)
                {
                    currentDocs.insert(doc.first); // doc.first 是 docid
                }

                // 计算交集
                set<int> tempResult;
                set_intersection(result.begin(), result.end(),
                                 currentDocs.begin(), currentDocs.end(),
                                 inserter(tempResult, tempResult.begin()));
                result = move(tempResult); // 更新 result
            }
        }
        else
        {
            // 如果某个词没有找到，直接返回空集
            return {};
        }
    }

    return result; // 返回同时包含所有词的文档ID集合
}