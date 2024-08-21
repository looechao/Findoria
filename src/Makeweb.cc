#include "../include/PagelibProcessor.h"

#include <iostream>


using std::cout;


int main()
{
    string path = "../data/material/web";
    vector<Item> cleandata;
    PagelibProcessor pagelibProcessor(path, cleandata);
    pagelibProcessor.readData();
    pagelibProcessor.cleanData();
    pagelibProcessor.storeOnDisk("../data/Pagelib.dat");
    return 0;
}

