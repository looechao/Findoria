#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

#include "../include/SplitTool.h"
#include "../include/DictProducer.h"

using std::endl;
using std::cout;
using std::string;
using std::cerr;


vector<string> inputFiles(const string& directoryPath){
    vector<string> files;
    DIR* dir = opendir(directoryPath.c_str());
    if(dir == nullptr){
        cerr << "Cannot open directory: " << directoryPath << "\n";
        return files;
    }
    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr)
    {
        string filePath = directoryPath + "/" + entry->d_name;
        struct stat fileStat;
        if(stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)){
            files.push_back(filePath);
        }
    }
    closedir(dir);
    return files;
}

void makeEnDict(){
    // 创建两个不同的SplitTool对象用于中英文的分词
    SplitTool* enCuttor = new SplitToolEn();
    vector<string> enMaterial = inputFiles("../data/material/english");
    //英文DictProducer, 用到了Singleton模式，对象的生命周期应该是静态局部变量的生命周期
    DictProducer enDictProducer = DictProducer(enMaterial, enCuttor);
    enDictProducer.setRaw();
    enDictProducer.cleanEnMaterial();
    enDictProducer.buildEnDict();
    enDictProducer.storeDict("../data/dict.dat");
}

void makeCnDict(){
    SplitTool* cnCuttor = new SplitToolCppJieba();
    vector<string> cnMaterial = inputFiles("../data/material/chinese");
    DictProducer cnDictProducer = DictProducer(cnMaterial, cnCuttor);
    cnDictProducer.setRaw();
    cnDictProducer.cleanCnMaterial();
    cnDictProducer.buildCnDict();
    cnDictProducer.storeDict("../data/dict.dat");
    cnDictProducer.createIndex("../data/dict.dat");
    cnDictProducer.storeIndexDict("../data/dictIndex.dat");
}


int main()
{
    makeEnDict();
    makeCnDict();
    return 0;
}

