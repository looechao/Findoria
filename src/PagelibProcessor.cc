#include "../include/PagelibProcessor.h"
#include <regex>

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
                item.link = getElementText(itemElement, "link");
                // 读取description并清洗description
                item.description = getElementText(itemElement, "description");
                item.description = removeHTMLTags(item.description);
                item.description = removeWhitespace(item.description);
                // 读取content并清洗content
                item.content = getElementText(itemElement, "content");
                item.content = removeHTMLTags(item.content);
                item.content = removeWhitespace(item.content);
                //_cleandata.emplace_back(item);放弃这个数据结构，直接写文件
                outFile << "<doc>\n";
                outFile << "    <docid>" << i + 1 << "</docid>\n"; // Assuming docid starts from 1
                outFile << "    <title>" << item.title << "</title>\n";
                outFile << "    <link>" << item.link << "</link>\n";
                outFile << "    <description>" << item.description << "</description>\n";
                outFile << "    <content>" << item.content << "</content>\n";
                outFile << "</doc>\n";
                i++; // docid从1开始计数
                itemElement = itemElement->NextSiblingElement("item");
            }
        }
    }
    //关掉路径，和目标存储文件
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