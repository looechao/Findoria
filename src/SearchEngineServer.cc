#include <iostream>

#include "../include/SearchEngineServer.h"

using std::cout;

void SearchEngineServer::start(unsigned short port){
    if (_httpserver.track().start(port) == 0)
    {
        _httpserver.list_routes();
        _waitGroup.wait();
        _httpserver.stop();
    }
    else
    {
        printf("SearchEngine Server Start Failed!\n");
    }
}


void SearchEngineServer::loadModules(){
    void loadDictionaryData();
    void loadDictionaryModule();
}


void SearchEngineServer::loadDictionaryData(){
    
}


void SearchEngineServer::loadDictionaryModule()
{
    // 注册词典库搜索的路由
    _httpserver.GET("/search/dictionary", std::bind(&SearchEngineServer::handle_dictionary_search, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void SearchEngineServer::loadWebPageModule()
{
    // 注册网页库搜索的路由
    _httpserver.GET("/search/webpage", std::bind(&SearchEngineServer::handle_webpage_search, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}