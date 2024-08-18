#ifndef __SplitTool_H__
#define __SplitTool_H__

#include <vector>
#include <iostream>

using std::string;
using std::vector;

class SplitTool
{
public:
    SplitTool() {}
    virtual ~SplitTool() {}

    virtual vector<string> cut(string str) = 0;

private:
};


class SplitToolCppJieba 
:public SplitTool
{
public:
    vector<string> cut(string str);
};

class SplitToolEn 
:public SplitTool
{
public:
    vector<string> cut(string str);
};

#endif // __SplitTool_H__
       //
