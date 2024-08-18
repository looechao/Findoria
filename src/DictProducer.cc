#include "../include/DictProducer.h"
#include <fstream>
#include <sstream>
#include <iostream>

using std::cerr;
using std::ostringstream;
using std::cout;

string readFileToString(string filePath){
    std::ifstream file(filePath);
    if(!file.is_open()){
        cerr << "Error opening file: " << filePath << "\n"; 
        return "";
    }

    ostringstream oss;
    oss << file.rdbuf();
    file.close();
    cout << "readfile to string success! \n";
    return oss.str();
}

void DictProducer::setRaw(){
    cout << "setRaw data:\n";
    for(auto & ele : this->_files){
        this->_raw.append(readFileToString(ele));
    }
    cout << this->_raw;
}