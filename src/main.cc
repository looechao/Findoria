#include <iostream>
#include "../include/SearchEngineServer.h"

using std::endl;
using std::cout;


int main()
{
    SearchEngineServer searchEngineServer(1);
    searchEngineServer.loadModules();
    searchEngineServer.start(1234);
    return 0;
}

