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
    ~SplitTool() {}

    vector<string> cut();

private:
};


class SplitToolCppJieba 
:public SplitTool
{
    vector<string> cut();
};

#endif // __SplitTool_H__
       //
