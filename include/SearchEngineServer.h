#ifndef __SearchEngineServer_H__
#define __SearchEngineServer_H__

#include <workflow/WFFacilities.h>
#include <wfrest/HttpServer.h>

class SearchEngineServer
{
public:
    SearchEngineServer(int cnt)
    : _waitGroup(cnt)
    {}
    ~SearchEngineServer() {}

    void start(unsigned short port);
    void loadModules();

private:
    //模块化的设计方式
    void loadDictionaryData();
    void loadWebPageData();
    void loadDictionaryModule();
    void loadWebPageModule();

    //处理函数
    void handle_dictionary_search(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series);
    void handle_webpage_search(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series);

private:
    WFFacilities::WaitGroup _waitGroup;
    wfrest::HttpServer _httpserver;
};

#endif // __SearchEngineServer_H__
       //
