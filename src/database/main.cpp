#include "database/dbman.h"
using namespace sql;

int main() {

    std::vector<sql::DBConnConfig> config;

    sql::DBConnConfig c;
    c.addr = "172.24.16.178:3306";
    c.driverName = "mysql";
    c.db = "yctest";
    c.passwd = "chronos";
    c.user = "chronos";
    config.push_back(c);

    DBMan::Instance()->Init(config);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}