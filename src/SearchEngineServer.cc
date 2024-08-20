#include <iostream>
#include <fstream>
#include <sstream>

#include "../include/SearchEngineServer.h"

using std::cout;
using std::getline;
using std::ifstream;
using std::map;
using std::pair;
using std::set;
using std::size_t;
using std::string;
using std::vector;

void SearchEngineServer::start(unsigned short port)
{
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

void SearchEngineServer::loadModules()
{
    loadDictionaryData(); // 加载词典和索引数据
}

void SearchEngineServer::loadDictionaryData()
{
    ifstream dictFile("../data/dict.dat");
    if (!dictFile.is_open())
    {
        std::cerr << "Error opening dictionary file." << std::endl;
        return;
    }
    string line;
    while (getline(dictFile, line))
    {
        std::istringstream iss(line);
        string word;
        int freq;
        if (iss >> word >> freq)
        {
            _wordRecomander._Dict.push_back(std::make_pair(word, freq));
        }
    }
    ifstream indexFile("../data/dictIndex.dat");
    if (!indexFile.is_open())
    {
        std::cerr << "Error opening index file." << std::endl;
        return;
    }
    // Read the index data
    while (std::getline(indexFile, line))
    {
        std::istringstream iss(line);
        std::string word;
        int id;
        // Get the word (first value)
        if (iss >> word)
        {
            // Read the following integers
            while (iss >> id)
            {
                // Insert the id into the set corresponding to the word
                _wordRecomander._Index[word].insert(id);
            }
        }
    }
}

void SearchEngineServer::loadWebPageData() {}


void SearchEngineServer::handle_webpage_search(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {}


void SearchEngineServer::handle_dictionary_search(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {}


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