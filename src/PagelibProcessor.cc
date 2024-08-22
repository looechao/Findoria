#include "../include/PagelibProcessor.h"
#include "../include/SplitTool.h"
#include <regex>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <iterator>
#include <mutex> // For mutex

#define LOGGER_LEVEL LL_WARN

using namespace tinyxml2;
using std::cerr;
using std::cout;
using std::ifstream;
using std::log2;
using std::ofstream;
using std::pair;
using std::regex;
using std::reverse_iterator;
using std::size_t;
using std::unordered_set;
using std::vector;

// 判断字符是否为中文字符
bool isChineseCharacter(const std::string &str, size_t pos)
{
    if (pos + 2 >= str.size())
        return false; // 需要至少3个字节

    unsigned char c1 = str[pos];
    unsigned char c2 = str[pos + 1];
    unsigned char c3 = str[pos + 2];

    // UTF-8 处理
    if (c1 < 0x80)
        return false; // ASCII范围内的字符

    // 汉字的 UTF-8 编码范围 (3字节)
    if (c1 >= 0xE4 && c1 <= 0xE9)
    {
        if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80)
        {
            return true; // 这范围包含了常用汉字
        }
    }
    return false;
}

// 判断字符是否为中文标点符号
bool isChinesePunctuation(const std::string &str, size_t pos)
{
    if (pos + 2 >= str.size())
        return false; // 需要至少3个字节

    unsigned char c1 = str[pos];
    unsigned char c2 = str[pos + 1];
    unsigned char c3 = str[pos + 2];

    // 检查中文标点符号范围
    if (c1 == 0xE3 && c2 == 0x80 && (c3 >= 0x80 && c3 <= 0xBF))
    {
        return true; // U+3000 至 U+303F 范围
    }
    if (c1 == 0xEF && c2 == 0xBC && (c3 >= 0x80 && c3 <= 0xBF))
    {
        return true; // U+FF00 至 U+FFEF 范围
    }
    if (c1 == 0xE2 && c2 == 0x80 && (c3 >= 0x90 && c3 <= 0xBF))
    {
        return true; // 部分 U+2000 至 U+206F 和 U+2010 至 U+201F 范围
    }
    if (c1 == 0xEF && c2 == 0xB8 && (c3 >= 0xB0 && c3 <= 0xBF))
    {
        return true; // U+FE30 至 U+FE4F 范围
    }

    return false;
}

// 判断字符是否为英文标点符号
bool isPunctuation(char c)
{
    return std::ispunct(static_cast<unsigned char>(c));
}

// 清理语料，为构建倒排索引做准备
std::string cleanMaterial(const std::string &raw)
{
    std::string result;
    size_t pos = 0;

    while (pos < raw.size())
    {
        if (isChineseCharacter(raw, pos))
        {
            // 保留汉字
            result.push_back(raw[pos]);
            result.push_back(raw[pos + 1]);
            result.push_back(raw[pos + 2]);
            pos += 3; // 跳过完整的汉字
        }
        else if (std::isalpha(static_cast<unsigned char>(raw[pos])) || std::isdigit(static_cast<unsigned char>(raw[pos])))
        {
            // 保留中英文字符和数字
            result.push_back(raw[pos]);
            pos++;
        }
        else if (isChinesePunctuation(raw, pos))
        {
            // 替换中文标点符号为空格
            result.push_back(' ');
            pos += 3; // 跳过中文标点符号（假设占用3个字节）
        }
        else if (isPunctuation(raw[pos]))
        {
            // 替换英文标点符号为空格
            result.push_back(' ');
            pos++; // 跳过英文标点符号
        }
        else
        {
            // 跳过空白符号
            pos++;
        }
    }

    // 去掉多余的空格
    std::string finalResult;
    bool lastWasSpace = false;

    for (char ch : result)
    {
        if (ch == ' ')
        {
            // 只在最后一个字符不是空格的情况下添加空格
            if (!lastWasSpace)
            {
                finalResult.push_back(ch);
                lastWasSpace = true;
            }
        }
        else
        {
            // 添加字符并重置空格标志
            finalResult.push_back(ch);
            lastWasSpace = false;
        }
    }

    // 去掉开头和结尾的空格
    if (!finalResult.empty() && finalResult.front() == ' ')
    {
        finalResult.erase(finalResult.begin());
    }
    if (!finalResult.empty() && finalResult.back() == ' ')
    {
        finalResult.pop_back();
    }

    return finalResult;
}

// 从 XML 元素中提取文本值的辅助函数
string getElementText(XMLElement *element, const char *name)
{
    if (element)
    {
        XMLElement *child = element->FirstChildElement(name);
        if (child && child->GetText())
        {
            return child->GetText();
        }
    }
    return "";
}

// Function to remove HTML tags from a string
// string removeHTMLTags(const string &input)
// {
//     regex htmlTagPattern("<.*?>");
//     return regex_replace(input, htmlTagPattern, "");
// }

string removeHTMLTags(const string &input)
{
    regex htmlTagPattern("<[^>]*>"); // Updated pattern to remove tags and their content
    return regex_replace(input, htmlTagPattern, "");
}

// Function to remove all whitespace characters
string removeWhitespace(const string &str)
{
    string result;
    result.reserve(str.size());
    for (char c : str)
    {
        if (!isspace(static_cast<unsigned char>(c)))
        {
            result.push_back(c);
        }
    }
    return result;
}

// Function to replace HTML entities
string replaceHTMLEntities(const string &input)
{
    string output = input;
    // 替换 &nbsp; 为普通空格
    size_t pos = output.find("&nbsp;");
    while (pos != string::npos)
    {
        output.replace(pos, 6, " "); // 替换为一个空格
        pos = output.find("&nbsp;", pos + 1);
    }
    // 其他常见 HTML 实体的替换
    pos = output.find("&lt;");
    while (pos != string::npos)
    {
        output.replace(pos, 4, "<");
        pos = output.find("&lt;", pos + 1);
    }
    pos = output.find("&gt;");
    while (pos != string::npos)
    {
        output.replace(pos, 4, ">");
        pos = output.find("&gt;", pos + 1);
    }
    // 添加更多的替换
    return output;
}

// 转义特殊字符
string escapeXML(const string &input)
{
    string output;
    for (char c : input)
    {
        switch (c)
        {
        case '&':
            output += "&amp;";
            break;
        case '<':
            output += "&lt;";
            break;
        case '>':
            output += "&gt;";
            break;
        case '"':
            output += "&quot;";
            break;
        case '\'':
            output += "&apos;";
            break;
        default:
            output += c;
            break;
        }
    }
    return output;
}

// 综合清洗函数
string cleanInput(const string &input)
{
    string cleaned = removeHTMLTags(input); // 去除 HTML 标签
    cleaned = replaceHTMLEntities(cleaned); // 替换 HTML 实体
    cleaned = removeWhitespace(cleaned);    // 去除多余空白
    return escapeXML(cleaned);              // 转义特殊字符
}

void PagelibProcessor::storeRawOnDisk(const string &filename)
{
    DIR *dir;
    struct dirent *entry;

    // 打开目录
    if ((dir = opendir(_directory.c_str())) == nullptr)
    {
        perror("opendir");
        return;
    }

    // 存储位置
    // doc id;
    int i = 0;
    ofstream outFile(filename);
    if (!outFile.is_open())
    {
        cerr << "Error: Could not open file " << filename << " for writing." << "\n";
        return;
    }
    outFile << "<docs>\n";
    // 遍历目录中的文件
    while ((entry = readdir(dir)) != nullptr)
    {
        string fileName = entry->d_name;
        if (fileName != "." && fileName != ".." && fileName.substr(fileName.find_last_of(".") + 1) == "xml")
        {
            string filePath = _directory + "/" + fileName;
            cout << "Processing file: " << filePath << "\n";

            XMLDocument doc;
            XMLError error = doc.LoadFile(filePath.c_str());
            if (error != XML_SUCCESS)
            {
                cout << "Failed to load XML file: " << filePath << "\n";
                continue;
            }

            XMLElement *root = doc.RootElement();
            if (!root)
            {
                cout << "No root element found in file: " << filePath << "\n";
                continue;
            }

            XMLElement *itemElement = root->FirstChildElement("channel")->FirstChildElement("item");
            if (!itemElement)
            {
                cout << "No item element found in file: " << filePath << "\n";
                continue;
            }

            while (itemElement)
            {
                Item item;
                item.title = getElementText(itemElement, "title");
                item.title = cleanInput(item.title);
                item.link = getElementText(itemElement, "link");
                // 读取description并清洗description
                item.description = getElementText(itemElement, "description");
                item.description = cleanInput(item.description);
                // 读取content并清洗content
                item.content = getElementText(itemElement, "content");
                item.content = cleanInput(item.content);
                //_cleandata.emplace_back(item);放弃这个数据结构，直接写文件
                outFile << "    <doc>\n";
                outFile << "        <docid>" << i + 1 << "</docid>\n"; // Assuming docid starts from 1
                outFile << "        <title>" << item.title << "</title>\n";
                outFile << "        <link>" << item.link << "</link>\n";
                outFile << "        <description>" << item.description << "</description>\n";
                outFile << "        <content>" << " " + item.content + " " << "</content>\n";
                outFile << "    </doc>\n";
                i++; // docid从1开始计数
                itemElement = itemElement->NextSiblingElement("item");
            }
        }
    }
    outFile << "</docs>\n";
    // 关掉路径，和目标存储文件
    closedir(dir);
    outFile.close();
    if (outFile.fail())
    {
        cerr << "Error: Failed to write to file " << filename << "." << "\n";
    }
    else
    {
        cout << "All raw Data successfully written to " << filename << "." << "\n";
    }
}

const char *const DICT_PATH = "../include/cppjieba/dict/jieba.dict.utf8";
const char *const HMM_PATH = "../include/cppjieba/dict/hmm_model.utf8";
const char *const IDF_PATH = "../include/cppjieba/dict/idf.utf8";
const char *const STOP_WORD_PATH = "../include/cppjieba/dict/stop_words.utf8";
simhash::Simhasher simhasher(DICT_PATH, HMM_PATH, IDF_PATH, STOP_WORD_PATH);

void PagelibProcessor::cutRedundantPage(const string &src, const string &des)
{
    int i = 0;
    int deletedNum = 0;
    ofstream outFile(des);
    outFile << "<docs>\n";

    // 2. 加载 XML 内容
    XMLDocument rawDoc;
    XMLError error = rawDoc.LoadFile(src.c_str());
    if (error != XML_SUCCESS)
    {
        cout << "Error loading XML file: " << src << "\n";
        cout << "Error code: " << error << "\n";
        cout << "Error message: " << rawDoc.ErrorIDToName(error) << "\n";
        return;
    }

    // 3. 获取根元素
    XMLElement *root = rawDoc.RootElement();
    if (!root)
    {
        cout << "No root element found in file: " << src << "\n";
        return;
    }

    cout << "Cutting Redundant Pages:\n";
    // 4. 遍历每个 doc 元素
    XMLElement *docElement = root->FirstChildElement("doc");
    while (docElement)
    {
        // 原有的处理逻辑
        Item item;
        item.title = getElementText(docElement, "title");
        item.link = getElementText(docElement, "link");
        item.description = getElementText(docElement, "description");
        item.content = getElementText(docElement, "content");

        uint64_t u;
        string s = item.description;
        if (s.empty())
        {
            s = item.content;
        }
        if (item.description.empty() && item.content.empty())
        {
            docElement = docElement->NextSiblingElement("doc");
            continue;
        }

        simhasher.make(s, 5, u);
        cout << "page id " << i << " generated simhash: " << u << "\n";

        bool foundEqual = false;
        for (const auto &hashValue : _simhasheLib)
        {
            if (simhash::Simhasher::isEqual(u, hashValue))
            {
                cout << "Found equal u: " << u << " in simhash library.\n";
                foundEqual = true;
                deletedNum++;
                break;
            }
        }

        if (foundEqual)
        {
            cout << "One page has been moved!\n";
            docElement = docElement->NextSiblingElement("doc");
            continue;
        }

        _simhasheLib.emplace_back(u);

        i++;
        // 添加到pagelib
        outFile << "    <doc>\n";
        outFile << "        <docid>" << i << "</docid>\n";
        outFile << "        <title>" << item.title << "</title>\n";
        outFile << "        <link>" << item.link << "</link>\n";
        outFile << "        <description>" << item.description << "</description>\n";
        outFile << "        <content>" << " " + item.content + " " << "</content>\n";
        outFile << "    </doc>\n";

        // 获取下一个 doc 元素
        docElement = docElement->NextSiblingElement("doc");

        // 每处理100个节点，清理内存并暂停
        if (i % 100 == 0)
        {
            cout << "Processed " << i << " documents. Cleaning memory and pausing...\n";
            rawDoc.Clear();               // 清理 XMLDocument 对象
            rawDoc.LoadFile(src.c_str()); // 重新加载文件
            root = rawDoc.RootElement();  // 重新获取根元素
            if (!root)
            {
                cout << "Error reloading XML file after cleanup.\n";
                return;
            }
            docElement = root->FirstChildElement("doc"); // 重新定位到当前处理的位置
            for (int j = 0; j < i; ++j)
            {
                docElement = docElement->NextSiblingElement("doc");
            }

            // 暂停一小段时间，让系统有机会进行其他操作
            sleep(2);
        }
    }

    outFile << "</docs>\n";
    outFile.close();
    cout << "All unique pages have been successfully written to " << des << "." << "\n"
         << deletedNum << " pages have been deleted\n";
}

void PagelibProcessor::createOffsetlib(const string &src)
{
    ifstream infile(src);
    if (!infile.is_open())
    {
        cerr << "Unable to open file: " << src << "\n";
        return;
    }

    string line;
    int docid = 0;
    int start = 0;
    int length = 0;
    bool inDoc = false;
    int docStartPos = 0;
    cout << "Processing offset for Page\n";

    while (getline(infile, line))
    {
        if (line.find("<doc>") != string::npos)
        {
            docid++;
            inDoc = true;
            docStartPos = infile.tellg();
            docStartPos -= line.size() + 1; // 减去当前行的长度和换行符
            continue;
        }

        if (line.find("</doc>") != string::npos)
        {
            inDoc = false;
            int docEndPos = infile.tellg();
            length = docEndPos - docStartPos;
            _offset[docid] = {docStartPos, length};
        }
    }
    cout << "All offset are successfully created\n";

    infile.close();
}

void PagelibProcessor::storeOffset(const string &filename) {
    cout << "Storing offset\n";
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Unable to open file for writing: " << filename << "\n";
        return;
    }

    // 写入map的大小
    size_t size = _offset.size();
    outfile << size << "\n"; // 使用文本方式写入大小

    // 逐个写入map的内容
    for (const auto &[docid, offsetPair] : _offset) {
        outfile << docid << " " << offsetPair.first << " " << offsetPair.second << "\n";
    }

    outfile.close();
    cout << "Offset is stored successfully\n";
}

// 测试功能，通过offset 找到文章内容
string getDocContent(const string &src, int docid, unordered_map<int, pair<int, int>> offset)
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

unordered_map<string, int> PagelibProcessor::calculateTermFrequency(const vector<string> &tokens)
{
    unordered_map<string, int> termFreq;
    for (const auto &token : tokens)
    {
        termFreq[token]++;
    }
    return termFreq;
}

unordered_map<string, int> globalDF; // 词汇和包含它的文章数量
int totalDocuments = 0;

// 词频计算的结构
struct WordStats
{
    int tf;        // 词频
    double weight; // 权重
};

SplitTool *cnCuttor = new SplitToolCppJieba(); // 用jieba的库

// 输出倒排索引的内容
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


void PagelibProcessor::buildInvertIndexMap()
{
    cout << "Generating InvertIndex...\n";
    SplitTool *cnCuttor = new SplitToolCppJieba(); // 用jieba的库

    // 用于存储每个文档的词频和权重
    vector<unordered_map<string, WordStats>> documentWordStats;

    // 遍历所有文档
    for (int i = 1; i < _offset.size() + 1; i++)
    {
        auto it = _offset.find(i);
        cout << "Processing page: " << it->first << "...\n";
        int docid = it->first;
        int start = it->second.first;
        int length = it->second.second;

        string content = getDocContent("../data/Pagelib.dat", docid, _offset);
        content = cleanMaterial(content);
        if (content.empty())
            continue;

        vector<string> tokens = cnCuttor->cut(content);
        unordered_map<string, WordStats> localTF; // 记录当前文档的词频

        // 统计当前文档中每个单词的词频
        for (const auto &token : tokens)
        {
            if (!token.empty())
            {
                localTF[token].tf++;
            }
        }
        cout << "Word frequency count is done!\n";

        // 将每个文档的词频信息存储到数组中
        documentWordStats.push_back(localTF);
        totalDocuments++;
    }
    cout << "All pages is processed!\n";

    // 更新全局文档频率 (DF)
    for (const auto &localTF : documentWordStats)
    {
        for (const auto &entry : localTF)
        {
            const string &word = entry.first;

            // 更新当前单词的文档频率
            if (globalDF.find(word) == globalDF.end())
            {
                globalDF[word] = 1; // 第一次见到该单词
            }
            else
            {
                globalDF[word]++; // 增加包含该词汇的文档数量
            }
        }
    }

    // 计算权重并更新倒排索引
    for (size_t i = 0; i < documentWordStats.size(); ++i)
    {
        const auto &localTF = documentWordStats[i];
        int docid = i+1; // 假设文档ID就是索引位置

        // 计算当前文档的权重总和
        double totalWeight = 0.0;
        unordered_map<string, double> weights;

        for (const auto &entry : localTF)
        {
            const string &word = entry.first;
            int tf = entry.second.tf;

            // 确保tf不为零
            if (tf > 0)
            {
                // 计算当前文档中的权重
                double idf = log2(static_cast<double>(totalDocuments) / (globalDF[word] + 1)); // 使用新的IDF公式

                // 确保idf为正值
                if (idf > 0)
                {
                    double weight = tf * idf; // 计算权重
                    weights[word] = weight;   // 存储每个单词的权重
                    totalWeight += weight;    // 更新总权重
                }
            }
        }

        // 归一化权重并更新倒排索引
        if (totalWeight > 0) // 确保总权重大于零以进行归一化
        {
            for (const auto &entry : weights)
            {
                const string &word = entry.first;
                double weight = entry.second / totalWeight; // 归一化
                _invertIndex[word].insert({docid, weight}); // 更新倒排索引
            }
        }
        else
        {
            cout << "Warning: Total weight for docID " << docid << " is zero. No normalization performed." << "\n";
        }
    }

    cout << "InvertIndex is generated!\n";
}

// 存储倒排索引到文件
void PagelibProcessor::storeInvertIndexMap(const string &filename)
{
    // 创建一个输出文件流
    ofstream outFile(filename, std::ios::binary);

    // 检查文件是否成功打开
    if (!outFile.is_open())
    {
        cerr << "Error opening file: " << filename << "\n";
        return;
    }

    // 写入map的大小
    size_t size = _invertIndex.size();
    outFile.write(reinterpret_cast<const char *>(&size), sizeof(size));

    // 遍历倒排索引并写入文件
    for (const auto &entry : _invertIndex)
    {
        const string &word = entry.first;                        // 当前单词
        const set<pair<int, double>> &docWeights = entry.second; // 当前单词对应的文档ID和权重集合

        // 写入单词长度
        size_t wordLength = word.size();
        outFile.write(reinterpret_cast<const char *>(&wordLength), sizeof(wordLength)); // 写入单词长度
        outFile.write(word.c_str(), wordLength);                                        // 写入单词本身

        // 写入docWeights的大小
        size_t docWeightsSize = docWeights.size();
        outFile.write(reinterpret_cast<const char *>(&docWeightsSize), sizeof(docWeightsSize)); // 写入文档权重集合的大小

        // 写入每个文档ID和权重
        for (const auto &docWeight : docWeights)
        {
            int docid = docWeight.first;                                            // 文档ID
            double weight = docWeight.second;                                       // 权重
            outFile.write(reinterpret_cast<const char *>(&docid), sizeof(docid));   // 写入文档ID
            outFile.write(reinterpret_cast<const char *>(&weight), sizeof(weight)); // 写入权重
        }
    }

    outFile.close();
    cout << "Invert index stored to " << filename << " successfully.\n";
}

// // 构建倒排索引 这个版本动态计算结果有问题
// void PagelibProcessor::buildInvertIndexMap()
// {
//     cout << "Generating InvertIndex...\n";
//     SplitTool *cnCuttor = new SplitToolCppJieba(); // 用jieba的库

//     for (auto it = _offset.begin(); it != _offset.end(); ++it)
//     {
//         cout << "Processing page: " << it->first << "...\n";
//         int docid = it->first;
//         int start = it->second.first;
//         int length = it->second.second;

//         string content = getDocContent("../data/Pagelib.dat", docid, _offset);
//         content = cleanMaterial(content);
//         if (content.empty())
//             continue;

//         vector<string> tokens = cnCuttor->cut(content);
//         unordered_map<string, WordStats> localTF; // 记录当前文档的词频

//         // 统计当前文档中每个单词的词频
//         for (const auto &token : tokens)
//         {
//             if (!token.empty())
//             {
//                 localTF[token].tf++;
//             }
//         }
//         cout << "Word frequency count is done!\n";

//         // 更新全局文档频率 (DF)
//         for (const auto &entry : localTF)
//         {
//             const string &word = entry.first;
//             int tf = entry.second.tf;

//             // 更新当前单词的文档频率
//             if (globalDF.find(word) == globalDF.end())
//             {
//                 globalDF[word] = 1; // 第一次见到该单词
//             }
//             else
//             {
//                 globalDF[word]++; // 增加包含该词汇的文档数量
//             }
//         }

//         // 计算当前文档的权重总和
//         double totalWeight = 0.0;
//         unordered_map<string, double> weights;

//         for (const auto &entry : localTF)
//         {
//             const string &word = entry.first;
//             int tf = entry.second.tf;

//             // 确保tf不为零
//             if (tf > 0)
//             {
//                 // 计算当前文档中的权重
//                 double idf = log2(static_cast<double>(totalDocuments) / (globalDF[word] + 1)); // 使用新的IDF公式

//                 // 确保idf为正值
//                 if (idf > 0)
//                 {
//                     double weight = tf * idf; // 计算权重
//                     weights[word] = weight;   // 存储每个单词的权重
//                     totalWeight += weight;    // 更新总权重
//                 }
//             }
//         }

//         // 归一化权重并更新倒排索引
//         if (totalWeight > 0)
//         { // 确保总权重大于零以进行归一化
//             for (const auto &entry : weights)
//             {
//                 const string &word = entry.first;
//                 double weight = entry.second / totalWeight; // 归一化
//                 _invertIndex[word].insert({docid, weight}); // 更新倒排索引
//             }
//         }
//         else
//         {
//             cout << "Warning: Total weight for docID " << docid << " is zero. No normalization performed." << "\n";
//         }

//         cout << "DF count is done!\n";
//         // 增加文档总数
//         totalDocuments++;
//         if (totalDocuments == 10)
//             break;
//     }
//     cout << "InvertIndex is generated!\n";
//     printInvertIndex(_invertIndex);
// }

// 开始计算simhash对文章去重 这个方法会卡死，内存占用过高
// void PagelibProcessor::cutRedundantPage(const string &src, const string &des)
// {
//     int i = 0;
//     int deletedNum = 0;
//     ofstream outFile(des);
//     outFile << "<docs>\n";
//     simhash::Simhasher simhasher(DICT_PATH, HMM_PATH, IDF_PATH, STOP_WORD_PATH);
//     // 2. 加载 XML 内容
//     XMLDocument rawDoc;
//     XMLError error = rawDoc.LoadFile(src.c_str());
//     if (error != XML_SUCCESS)
//     {
//         cout << "Error loading XML file: " << src << "\n";
//         cout << "Error code: " << error << "\n";
//         cout << "Error message: " << rawDoc.ErrorIDToName(error) << "\n";
//         return;
//     }

//     // 3. 获取根元素
//     XMLElement *root = rawDoc.RootElement();
//     if (!root)
//     {
//         cout << "No root element found in file: " << src << "\n";
//         return;
//     }

//     cout << "Cutting Redundant Pages:\n";
//     // 4. 遍历每个 doc 元素
//     XMLElement *docElement = root->FirstChildElement("doc");
//     while (docElement)
//     {
//         Item item;
//         item.title = getElementText(docElement, "title");
//         item.link = getElementText(docElement, "link");
//         // 读取description并清洗description
//         item.description = getElementText(docElement, "description");
//         // 读取content并清洗content
//         item.content = getElementText(docElement, "content");

//         uint64_t u;
//         string s = item.description;
//         if (s.empty())
//         {
//             s = item.content;
//         }
//         if (item.description.empty() && item.content.empty())
//         {
//             continue;
//         }
//         // 计算simhash数值
//         simhasher.make(s, 5, u);
//         cout <<"page id " << i << "generated simhash: " << u << "\n";

//         // 检查 u 是否与 _simhashLib 中的元素相等
//         bool foundEqual = false; // 标记是否找到相等的 simhash

//         for (const auto &hashValue : _simhasheLib)
//         {
//             if (simhash::Simhasher::isEqual(u, hashValue))
//             {
//                 cout << "Found equal u: " << u << " in simhash library.\n";
//                 foundEqual = true;
//                 docElement = docElement->NextSiblingElement("doc");
//                 deletedNum++;
//                 break; // 找到相等，跳出循环
//             }
//         }
//         cout << "Comparison finished! \n";

//         if (foundEqual)
//         {
//             cout << "One page has been moved!\n"; // 如果找到相等的值，则跳出 while 循环
//             continue;
//         }

//         _simhasheLib.emplace_back(u);
//         cout << "Insert finished! \n";

//         i++;
//         // 添加到pagelib
//         outFile << "    <doc>\n";
//         outFile << "        <docid>" << i << "</docid>\n"; // Assuming docid starts from 1
//         outFile << "        <title>" << item.title << "</title>\n";
//         outFile << "        <link>" << item.link << "</link>\n";
//         outFile << "        <description>" << item.description << "</description>\n";
//         outFile << "        <content>" << " " + item.content + " " << "</content>\n";
//         outFile << "    </doc>\n";
//         cout << "Write finished! \n";

//         // 获取下一个 doc 元素
//         docElement = docElement->NextSiblingElement("doc");
//         cout << "next node found\n";
//     }
//     outFile << "</docs>\n";
//     outFile.close();
//     cout << "All unique pages has been successfully written to " << src << "." << "\n" << deletedNum << " pages has been deleted\n";
// }