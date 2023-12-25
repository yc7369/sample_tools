#include "cppconn/resultset.h"
#include "database/dbman.h"
#include <iostream>
#include <sstream>
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
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto db = DBMan::Instance()->GetDB();
    if (db) {
        std::ostringstream ss;
        ss << "SELECT TABLE_NAME FROM information_schema.tables WHERE table_schema = '";
        ss << c.db << "'";
        SQLError ret_code = sql::SQLError::SUCCESS;
        auto stmt = db->Prepared(ss.str(), ret_code);
        if (stmt) {
            auto res = std::shared_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res) {
                while (res->next()) {
                    std::string n1 = res->getString("TABLE_NAME");
                    std::cout << n1.c_str() << std::endl;
                }
            }
        }
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}