#include "linker_impl_server.h"
#include "qtp_msg.h"
using namespace linker;
using namespace tcp;

int main() {
    auto s = std::make_shared<XTcpServer>();
    s->init("{}");

    ConnectionParams conn_params;
    conn_params.address = "172.17.0.4";
    conn_params.port = "8888";
    s->Start(conn_params);
    s->Run();

    return 0;
}