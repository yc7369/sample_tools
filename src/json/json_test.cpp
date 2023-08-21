#include "json.hpp"
#include <iostream>
#include "defines.h"
using namespace nlohmann;

int main(){
    json doc;
    doc["a"] = "b";
    json d1;
    for(int i = 0; i < 10; ++i){
        d1.push_back(i);
    }
    doc["array"] = d1;

    std::cout<<doc.dump()<<std::endl;

    std::string data = R"({"a":"b","array":[0,1,2,3,4,5,6,7,8,9]})";
    json d;
    try
    {
        auto d = json::parse(data);

        std::cout<<d["a"]<<std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    if(likely(typeid(d) == typeid(json))){
        std::cout<<"json type"<<std::endl;
    }else if(likely(typeid(d) == typeid(std::string))){
        std::cout<<"std::string type"<<std::endl;
    }
    
    return 0;
}