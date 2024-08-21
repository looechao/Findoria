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

void PagelibProcessor::readData()
{
    DIR *dir;
    struct dirent *entry;

    // 打开目录
    if ((dir = opendir(_directory.c_str())) == nullptr)
    {
        perror("opendir");
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
                item.description = getElementText(itemElement, "description");
                item.content = getElementText(itemElement, "content");

                _cleandata.emplace_back(item);
                itemElement = itemElement->NextSiblingElement("item");
            }
        }
    }

    closedir(dir);
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

// Method to clean data
void PagelibProcessor::cleanData()
{
    cout << "Cleaning data\n";
    for (Item &item : _cleandata)
    {
        item.description = removeHTMLTags(item.description);
        item.description = removeWhitespace(item.description);
    }
    cout << "Data successfully cleaned up!\n";
}

void PagelibProcessor::storeOnDisk(const string &filename)
{
    ofstream outFile(filename);
    if (!outFile.is_open())
    {
        cerr << "Error: Could not open file " << filename << " for writing." << "\n";
        return;
    }

    for (size_t i = 0; i < _cleandata.size(); ++i)
    {
        const Item &item = _cleandata[i];
        outFile << "<doc>\n";
        outFile << "    <docid>" << i + 1 << "</docid>\n"; // Assuming docid starts from 1
        outFile << "    <title>" << item.title << "</title>\n";
        outFile << "    <link>" << item.link << "</link>\n";
        outFile << "    <description>" << item.description << "</description>\n";
        outFile << "    <content>" << item.content << "</content>\n";
        outFile << "</doc>\n";
    }

    outFile.close();
    if (outFile.fail())
    {
        cerr << "Error: Failed to write to file " << filename << "." << "\n";
    }
    else
    {
        cout << "Data successfully written to " << filename << "." << "\n";
    }
}
