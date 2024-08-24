#include <iostream>
#include <fstream>
#include <sstream>

#include "../include/nlohmann/json.hpp"
#include "../include/SearchEngineServer.h"

using std::cerr;
using std::cout;
using std::getline;
using std::ifstream;
using std::map;
using std::pair;
using std::set;
using std::size_t;
using std::string;
using std::vector;
using namespace tinyxml2;

string urlDecode(const std::string &encoded)
{
    std::ostringstream decoded;
    decoded.fill('0');
    std::string::size_type len = encoded.length();
    for (std::string::size_type i = 0; i < len; ++i)
    {
        if (encoded[i] == '%')
        {
            if (i + 2 < len)
            {
                std::string hex = encoded.substr(i + 1, 2);
                int value = 0;
                std::istringstream(hex) >> std::hex >> value;
                decoded << static_cast<char>(value);
                i += 2;
            }
        }
        else if (encoded[i] == '+')
        {
            decoded << ' ';
        }
        else
        {
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
    loadWebPageData();      // 加载pagelib的索引和偏移量
    loadDictionaryData();   // 加载词典和索引数据
    loadStaticSource();     // 加载静态资源
    loadDictionaryModule(); // 加载推荐词模块
    loadWebPageModule();    // 加载网页推荐模块
}

void SearchEngineServer::loadStaticSource()
{
    _httpserver.GET("/home", [](const wfrest::HttpReq *, wfrest::HttpResp *resp)
                    { resp->File("../static/view/home.html"); });
    _httpserver.GET("/static/assets/logo.png", [](const wfrest::HttpReq *, wfrest::HttpResp * resp){
        resp->File("../static/assets/logo.png");
    });
}

void SearchEngineServer::loadDictionaryData()
{
    _wordRecomander.loadDict("../data/dict.dat");
    _wordRecomander.loadIndex("../data/dictIndex.dat");
}

void SearchEngineServer::loadWebPageData()
{
    _webPageSearcher.loadOffset("../data/offset.dat");
    _webPageSearcher.loadInvertIndex("../data/invertIndex.dat");
}

void SearchEngineServer::handle_webpage_search(const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series)
{
    string encoded_query = req->query("sentence");
    string decoded_query = urlDecode(encoded_query);
    cout << "decoded: " << decoded_query << "\n";

    if (decoded_query.empty())
    {
        cerr << "Missing query parameter 'sentence' \n";
        return;
    }

    set<int> docs = _webPageSearcher.queryDocuments(decoded_query);
    unordered_map<string, double> queryWeights = _webPageSearcher.calculateTFIDF(_webPageSearcher.splitSentence(decoded_query), docs.size());
    for (auto &word : queryWeights)
    {
        cout << "word: " << word.first << " weight: " << word.second << " \n";
    }

    vector<pair<int, double>> rankedDocs = _webPageSearcher.rankDocs(docs, queryWeights);
    // 只保留前十个网页
    if (rankedDocs.size() > 10)
    {
        rankedDocs.resize(10);
    }
    nlohmann::json jsonResponse;
    nlohmann::json documentsArray;
    int i = 0;
    cout << "test\n";
    string webContent = _webPageSearcher.getDocContent("../data/Pagelib.dat", 3606, _webPageSearcher._offset);
    XMLDocument xmlDoc;
    xmlDoc.Parse(webContent.c_str());
    XMLElement *docElement = xmlDoc.RootElement();
    cout << "test done!";
    for(auto &doc : rankedDocs)
    {
        string webContent = _webPageSearcher.getDocContent("../data/Pagelib.dat", doc.first, _webPageSearcher._offset);
        XMLDocument xmlDoc;
        xmlDoc.Parse(webContent.c_str());
        XMLElement *docElement = xmlDoc.RootElement();
        i++;
        if (!docElement)
        {
            cerr << "Invalid XML format \n"
                 << i << "\n";
            continue;
        }
        const char *docid = docElement->FirstChildElement("docid")->GetText();
        const char *title = docElement->FirstChildElement("title")->GetText();
        const char *link = docElement->FirstChildElement("link")->GetText();
        const char *description = docElement->FirstChildElement("description")->GetText();
        const char *content = docElement->FirstChildElement("content")->GetText();

        // 构造JSON对象
        nlohmann::json document;
        document["docid"] = docid ? docid : "";
        document["title"] = title ? title : "";
        document["link"] = link ? link : "";
        document["description"] = description ? description : "";
        document["content"] = content ? content : "";
        documentsArray.push_back(document); // 添加到文档数组中
    }

    jsonResponse["documents"] = documentsArray; // 将文档数组添加到响应中
    // 一次性发送完整的 JSON 响应
    resp->Json(jsonResponse.dump());
    cout << "\n";
}

void SearchEngineServer::handle_dictionary_search(const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series)
{
    string encoded_query = req->query("wd");
    string decoded_query = urlDecode(encoded_query);
    cout << "decoded: " << decoded_query << "\n";

    if (decoded_query.empty())
    {
        cerr << "Missing query parameter 'wd' \n";
        return;
    }
    vector<string> splitedQuery = _wordRecomander.split_query(decoded_query);
    vector<CandidateResult> recomandWord = _wordRecomander.generateCandidates(splitedQuery, decoded_query);
    recomandWord = _wordRecomander.sortCandidates(recomandWord);
    nlohmann::json jsonResponse;
    for (auto &it : recomandWord)
    {
        jsonResponse["words"].push_back(it._word); // 添加 word 到 JSON 数组
        // cout << "recomandWord: " << it._word << " freq:" << it._freq << " editDistance: "<< it._dist << "\n";
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