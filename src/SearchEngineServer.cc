#include <iostream>
#include <fstream>
#include <sstream>

#include "../include/nlohmann/json.hpp"
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
using std::cerr;

string urlDecode(const std::string &encoded) {
    std::ostringstream decoded;
    decoded.fill('0');
    std::string::size_type len = encoded.length();
    for (std::string::size_type i = 0; i < len; ++i) {
        if (encoded[i] == '%') {
            if (i + 2 < len) {
                std::string hex = encoded.substr(i + 1, 2);
                int value = 0;
                std::istringstream(hex) >> std::hex >> value;
                decoded << static_cast<char>(value);
                i += 2;
            }
        } else if (encoded[i] == '+') {
            decoded << ' ';
        } else {
            decoded << encoded[i];
        }
    }
    return decoded.str();
}

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
    loadStaticSource(); // 加载静态资源
    loadDictionaryModule();
}


void SearchEngineServer::loadStaticSource(){
    _httpserver.GET("/home", [](const wfrest::HttpReq *, wfrest::HttpResp *resp)
    { resp->File("../static/view/home.html"); });
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


void SearchEngineServer::handle_dictionary_search(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {
    string encoded_query = req->query("wd");
    string decoded_query = urlDecode(encoded_query);
    cout <<  "decoded: " << decoded_query << "\n";

    if(decoded_query.empty()){
        cerr << "Missing query parameter 'wd' \n";
        return;
    }
    vector<string> splitedQuery = _wordRecomander.split_query(decoded_query);
    vector<CandidateResult> recomandWord = _wordRecomander.generateCandidates(splitedQuery, decoded_query);
    recomandWord = _wordRecomander.sortCandidates(recomandWord);
    nlohmann::json jsonResponse;
    for(auto &it : recomandWord){
        jsonResponse["words"].push_back(it._word); // 添加 word 到 JSON 数组
        //cout << "recomandWord: " << it._word << " freq:" << it._freq << " editDistance: "<< it._dist << "\n";
    }
    resp->String(jsonResponse.dump()); // 将 JSON 对象转换为字符串并写入响应
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