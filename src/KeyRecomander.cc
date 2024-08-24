#include "KeyRecomander.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using std::cerr;
using std::cout;
using std::getline;
using std::ifstream;
using std::ios;
using std::pair;
using std::set;
using std::size_t;
using std::sort;
using std::string;
using std::vector;

// 1. 求取一个字符占据的字节数
size_t nBytesCode(const char ch);

// 2. 求取一个字符串的字符长度
size_t length(const string &str);

// 3. 中英文通用的最小编辑距离算法
int editDistance(const string &lhs, const string &rhs);

size_t nBytesCode(const char ch)
{
    if (ch & (1 << 7))
    {
        int nBytes = 1;
        for (int idx = 0; idx != 6; ++idx)
        {
            if (ch & (1 << (6 - idx)))
            {
                ++nBytes;
            }
            else
                break;
        }
        return nBytes;
    }
    return 1;
}

int triple_min(const int &a, const int &b, const int &c)
{
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

size_t length(const std::string &str)
{
    std::size_t ilen = 0;
    for (std::size_t idx = 0; idx != str.size(); ++idx)
    {
        int nBytes = nBytesCode(str[idx]);
        idx += (nBytes - 1);
        ++ilen;
    }
    return ilen;
}

// 最小编辑距离算法
int editDistance(const std::string &lhs, const std::string &rhs)
{
    // 计算最小编辑距离-包括处理中英文
    size_t lhs_len = length(lhs);
    size_t rhs_len = length(rhs);
    int editDist[lhs_len + 1][rhs_len + 1];
    for (size_t idx = 0; idx <= lhs_len; ++idx)
    {
        editDist[idx][0] = idx;
    }
    for (size_t idx = 0; idx <= rhs_len; ++idx)
    {
        editDist[0][idx] = idx;
    }

    std::string sublhs, subrhs;
    for (std::size_t dist_i = 1, lhs_idx = 0; dist_i <= lhs_len; ++dist_i,
                     ++lhs_idx)
    {
        size_t nBytes = nBytesCode(lhs[lhs_idx]);
        sublhs = lhs.substr(lhs_idx, nBytes);
        lhs_idx += (nBytes - 1);
        for (std::size_t dist_j = 1, rhs_idx = 0;
             dist_j <= rhs_len; ++dist_j, ++rhs_idx)
        {
            nBytes = nBytesCode(rhs[rhs_idx]);
            subrhs = rhs.substr(rhs_idx, nBytes);
            rhs_idx += (nBytes - 1);
            if (sublhs == subrhs)
            {
                editDist[dist_i][dist_j] = editDist[dist_i - 1][dist_j -
                                                                1];
            }
            else
            {
                editDist[dist_i][dist_j] =
                    triple_min(editDist[dist_i][dist_j - 1] + 1,
                               editDist[dist_i - 1][dist_j] + 1,
                               editDist[dist_i - 1][dist_j - 1] + 1);
            }
        }
    }
    return editDist[lhs_len][rhs_len];
}

//---------------------------------------------------------------------------

vector<CandidateResult> KeyRecomander::generateCandidates(vector<string> splited_words, string query)
{
    set<int> resultSet;
    for (const auto &word : splited_words)
    {
        auto it = _Index.find(word);
        if (it != _Index.end())
        {
            if (resultSet.empty())
            {
                resultSet.insert(it->second.begin(), it->second.end());
            }
        }
    }

    vector<CandidateResult> recommendations;
    for (int idx : resultSet)
    {
        if (idx >= 0 && idx < _Dict.size())
        {
            if (_Dict[idx].first.size() >= query.size())
            {
                CandidateResult tmp(_Dict[idx].first, _Dict[idx].second, editDistance(_Dict[idx].first, query));
                recommendations.emplace_back(tmp);
            }
        }
    }
    cout << "recomeneded words are generated! \n";
    return recommendations;
}

vector<CandidateResult> KeyRecomander::sortCandidates(vector<CandidateResult> candidates)
{
    // 使用自定义比较函数进行排序
    sort(candidates.begin(), candidates.end(), [](const CandidateResult &a, const CandidateResult &b)
         {
             // 按编辑距离升序排序
             if (a._dist != b._dist)
             {
                 return a._dist < b._dist; // 低编辑距离排在前面
             }
             // 若编辑距离相同，则按频率降序排序
             return a._freq > b._freq; // 高频率排在前面
         });
    if (candidates.size() > 10)
    {
        candidates.resize(10); // 调整 vector 的大小
    }
    return candidates; // 返回调整后的 vector
}

vector<string> KeyRecomander::split_query(string query)
{
    vector<string> keywords;
    string current;

    for (size_t i = 0; i < query.size(); ++i)
    {
        char c = query[i];
        if ((unsigned char)c >= 0x80)
        { // 非ASCII字符（可能是中文字符）
            // 处理中文字符（假定UTF-8编码，中文字符占用3个字节）
            string chinese_char;
            chinese_char += c;
            if (i + 1 < query.size())
                chinese_char += query[i + 1];
            if (i + 2 < query.size())
                chinese_char += query[i + 2];
            keywords.push_back(chinese_char);
            i += 2; // 跳过下两个字节（中文字符已经处理）
        }
        else if (isalnum(c))
        {
            // 英文字符或数字
            keywords.push_back(string(1, c));
        }
        else
        {
            // 跳过标点符号或空白字符
            continue;
        }
    }
    cout << "query is splited!\n";
    return keywords;
}

void KeyRecomander::loadDict(const string &filename)
{
    ifstream dictFile(filename);
    if (!dictFile.is_open())
    {
        std::cerr << "Error opening dictionary file." << std::endl;
        return;
    }
    string line;
    while (getline(dictFile, line))
    {
        std::istringstream iss(line);
        string word;
        int freq;
        if (iss >> word >> freq)
        {
            _Dict.push_back(std::make_pair(word, freq));
        }
    }
}

void KeyRecomander::loadIndex(const string &filename)
{
    ifstream indexFile(filename);
    if (!indexFile.is_open())
    {
        std::cerr << "Error opening index file." << std::endl;
        return;
    }
    // Read the index data
    string line;
    while (getline(indexFile, line))
    {
        std::istringstream iss(line);
        std::string word;
        int id;
        // Get the word (first value)
        if (iss >> word)
        {
            // Read the following integers
            while (iss >> id)
            {
                // Insert the id into the set corresponding to the word
                _Index[word].insert(id);
            }
        }
    }
}