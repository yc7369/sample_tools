#include "json.hpp"
#include <iostream>
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
    return 0;
}