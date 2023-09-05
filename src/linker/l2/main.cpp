#include "mybase/timerman.h"
#include <vector>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include "signal_server.h"
#include "zlog/ztp_log.h"

#ifndef BUILD_VERSION
#define BUILD_VERSION "v1.0.0"
#endif

const char* __APP_VERSION = BUILD_VERSION;
const char* _APP_VERSION = __APP_VERSION + 1;

int main(int argc, char** argv) {
    std::string config_file = "config.json";
    int result;
    while ((result = getopt(argc, argv, "vc:")) != -1) {
        switch (result) {
        case 'c':
            config_file = optarg;
            break;
        case 'v':
            std::cout << _APP_VERSION << std::endl;
            exit(0);
            break;
        }
    }
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == 0) {
        std::cout << "SIGPIPE ignore" << std::endl;
    } else {
        std::cerr << "SIGPIPE ignore" << std::endl;
    }

    ztp::setup_log_system("./logs", BASE_LOGGER_NAME, 0, BASE_LOGGER_NAME);

    char* pwd = getcwd(NULL, 0);
    ZLOG_INFO("Using config file {}", config_file);
    ZLOG_INFO("SIGNAL_SERVER_VERSION: {}, work dir: {}", _APP_VERSION, pwd);

    mybase::TimerMan::instance()->run();

    sigsrv::SignalServer m;
    if (!m.Init()) {
        ZLOG_ERROR("signal server init failed");
        return -1;
    }
    tcp::ConnectionParams conn_params;
    conn_params.address = "172.17.0.4";
    conn_params.port = "8888";
    m.Start(conn_params);
    m.Run();
    return 0;
}