#include "../include/PagelibProcessor.h"

#include <iostream>


using std::cout;


int main()
{
    string path = "../data/material/web";
    PagelibProcessor pagelibProcessor(path);
    pagelibProcessor.storeRawOnDisk("../data/RawPagelib.dat");
    return 0;
}

