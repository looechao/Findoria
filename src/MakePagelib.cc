#include "../include/PagelibProcessor.h"

#include <iostream>


using std::cout;
using std::ifstream;
using std::ios;
using std::cerr;
using std::size_t;
using std::unordered_map;

// 测试功能，通过offset 找到文章内容
// string getDocContent(const string &src, int docid, unordered_map<int, pair<int, int>> offset)
// {
//     ifstream infile(src, std::ios::binary);
//     if (!infile.is_open())
//     {
//         cerr << "Unable to open file: " << src << "\n";
//         return "";
//     }

//     if (offset.find(docid) == offset.end())
//     {
//         cerr << "DocID not found: " << docid << "\n";
//         return "";
//     }

//     auto [start, length] = offset[docid];

//     infile.seekg(start, std::ios::beg);
//     string content(length, '\0');
//     infile.read(&content[0], length);

//     infile.close();
//     return content;
// }

// 测试读取unordered_map的数据结构
unordered_map<int, pair<int, int>> loadOffset(const string& filename) {
    unordered_map<int, pair<int, int>> offset;
    ifstream infile(filename, ios::binary);
    if (!infile.is_open()) {
        cerr << "Unable to open file for reading: " << filename << "\n";
        return offset;
    }

    // 读取map的大小
    size_t size;
    infile.read(reinterpret_cast<char*>(&size), sizeof(size));

    // 逐个读取map的内容
    for (size_t i = 0; i < size; ++i) {
        int docid;
        int start, length;
        infile.read(reinterpret_cast<char*>(&docid), sizeof(docid));
        infile.read(reinterpret_cast<char*>(&start), sizeof(start));
        infile.read(reinterpret_cast<char*>(&length), sizeof(length));
        offset[docid] = {start, length};
    }

    infile.close();
    return offset;
}

void removeRawData(){
    // 删除原始数据文件
    if (remove("../data/RawPagelib.dat") != 0) {
        std::cerr << "Error: Could not delete RawPagelib.dat" << "\n";
    } else {
        std::cout << "Successfully deleted RawPagelib.dat" << "\n";
    }
}

int main()
{
    string path = "../data/material/web";
    PagelibProcessor pagelibProcessor(path);
    pagelibProcessor.storeRawOnDisk("../data/RawPagelib.dat");
    pagelibProcessor.cutRedundantPage("../data/RawPagelib.dat", "../data/Pagelib.dat");
    removeRawData();
    pagelibProcessor.createOffsetlib("../data/Pagelib.dat");
    pagelibProcessor.storeOffset("../data/offset.dat");
    pagelibProcessor.buildInvertIndexMap();
    pagelibProcessor.storeInvertIndexMap("../data/inverIndex.dat");
    return 0;
}

