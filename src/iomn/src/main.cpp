#include "unix_socket/metric.h"
#include <unistd.h>

int main() {
    auto p = Metric::inst();
    p->run("test_8106");
    p->push_status("start", "success");
    int count = 0;
    while (true) {
        p->push_count("times");
        p->push_summary("summary", count++, { 1, 2, 5, 10, 100 });
        p->push_summary("summary2", count++, { 1, 20, 50 });
        p->push_guage("guagage", count);

        usleep(1000);
    }
}