#include "../include/PagelibProcessor.h"
#include <regex>
#include <thread>
#include <chrono>

#define LOGGER_LEVEL LL_WARN

using namespace tinyxml2;
using std::cerr;
using std::cout;
using std::ofstream;
using std::regex;
using std::size_t;


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
string removeHTMLTags(const string &input)
{
    regex htmlTagPattern("<.*?>");
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

void PagelibProcessor::cutRedundantPage(const string &src, const string &des)
{
    int i = 0;
    int deletedNum = 0;
    ofstream outFile(des);
    outFile << "<docs>\n";
    simhash::Simhasher simhasher(DICT_PATH, HMM_PATH, IDF_PATH, STOP_WORD_PATH);
    
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
        cout <<"page id " << i << " generated simhash: " << u << "\n";

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
            rawDoc.Clear(); // 清理 XMLDocument 对象
            rawDoc.LoadFile(src.c_str()); // 重新加载文件
            root = rawDoc.RootElement(); // 重新获取根元素
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


void PagelibProcessor::createOffsetlib(){
    
}


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