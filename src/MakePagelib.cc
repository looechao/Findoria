#include "../include/PagelibProcessor.h"

#include <iostream>


using std::cout;

void removeRawData(){
    // 删除原始数据文件
    if (remove("../data/RawPagelib.dat") != 0) {
        std::cerr << "Error: Could not delete RawPagelib.dat" << std::endl;
    } else {
        std::cout << "Successfully deleted RawPagelib.dat" << std::endl;
    }
}

int main()
{
    string path = "../data/material/web";
    PagelibProcessor pagelibProcessor(path);
    pagelibProcessor.storeRawOnDisk("../data/RawPagelib.dat");
    pagelibProcessor.cutRedundantPage("../data/RawPagelib.dat", "../data/Pagelib.dat");
    removeRawData();
    return 0;
}

