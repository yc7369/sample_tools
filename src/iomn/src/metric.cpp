#include "metric.h"
#ifndef _MSC_VER
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

#define QLEN 10

static const std::string socket_dir = "/tmp/sds/";

int serv_listen(const char* name) {
    int fd, len, err, rval;
    struct sockaddr_un un;
    /* create a UNIX domain stream socket */
    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("error socket:");
        return (-1);
    }
    unlink(name); /* in case it already exists 否则bind的时候会出错*/
    /* fill in socket address structure */
    memset(&un, 0, sizeof(un));
    un.sun_family = PF_UNIX;
    strcpy(un.sun_path, name);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
    /* bind the name to the descriptor 会创建name*/
    if (bind(fd, (struct sockaddr*)&un, len) < 0) {
        rval = -2;
        perror("bind:");
        goto errout;
    }
    if (listen(fd, QLEN) < 0) { /* tell kernel we're a server */
        rval = -3;
        perror("listen:");
        goto errout;
    }
    return (fd);
errout:
    err = errno;
    close(fd);
    errno = err;
    return (rval);
}

int serv_accept(int listenfd, uid_t* uidptr) {
    int clifd;
    socklen_t len;
    struct sockaddr_un un;
    len = sizeof(un);
    if ((clifd = accept(listenfd, (struct sockaddr*)&un, &len)) < 0)
        return (-1); /* often errno=EINTR, if signal caught */
    return clifd;
}

Metric::Metric(/* args */) {}

int64_t get_ms() {
    using namespace std;
    auto timeNow = std::chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch());
    return timeNow.count();
}
void Metric::run(const std::string& app_tag) {

    app_tag_ = app_tag;

    is_running_ = true;

    signal(SIGPIPE, SIG_IGN);

    t1_ = std::make_shared<std::thread>([&]() {
        int listen_fd, connect_fd;
        uid_t cuid;

        std::string socket_name = socket_dir + app_tag_;

        struct stat s;
        if (0 == stat(socket_dir.data(), &s) && !S_ISDIR(s.st_mode)) {
            std::cerr << " Not a dir,please remove " << socket_dir << std::endl;
            is_running_ = false;
            return;
        } else {
            system("mkdir -p /tmp/sds/");
        }

        listen_fd = serv_listen(socket_name.data());
        std::cout << "start listen at:" << socket_name << std::endl;

        if (listen_fd < 0) {
            perror(socket_name.data());
            is_running_ = false;
            return;
        }
        while (is_running_) {
            connect_fd = serv_accept(listen_fd, &cuid);

            if (!is_running_) {
                return;
            }
            if (connect_fd < 0) {
                perror("accept");
                return;
            }

            gen_and_write_output(connect_fd);
            close(connect_fd);
        }
    });

    t2_ = std::make_shared<std::thread>([&]() {
        int64_t time_last = get_ms();
        summary_current_index_ = 0;
        while (is_running_) {
            std::shared_ptr<metric> d;
            auto now = get_ms();
            bool update_index = false;
            for (auto& i : summary_map_) {
                if (now - 1000 >= time_last) {
                    auto tmp_index = summary_current_index_ + 1;
                    if (tmp_index >= summary_count) {
                        tmp_index = 0;
                    }
                    i.second[tmp_index] = 0;
                    update_index = true;
                }
            }

            std::lock_guard lock(map_mutex);

            if (update_index) {
                summary_current_index_++;
                if (summary_current_index_ >= summary_count) {
                    summary_current_index_ = 0;
                }
                time_last = now;
            }

            if (!q_.try_dequeue(d)) {
                std::this_thread::sleep_for(std::chrono::microseconds(200));
                continue;
            }

            // std::cout << "dequeue msg:" << (int)d->type << " data:" << d->key
            //           << " d->strvalue:" << d->str_value << " d->int_value:" <<
            //           d->int_value
            //           << std::endl;

            auto update_summary = [&]() {
                if (summary_map_.find(d->key) == summary_map_.end()) {
                    summary_map_[d->key] = { 0 };
                    summary_type_map_[d->key] = d->interval;
                }
                summary_map_[d->key][summary_current_index_] += d->int_value;
                summary_total_map_[d->key] += d->int_value;
            };

            switch (d->type) {
            case METRIC_TYPE::count:
                count_map_[d->key]++;
                break;
            case METRIC_TYPE::gugae:
                guage_map_[d->key] = d->int_value;
                break;
            case METRIC_TYPE::histogram:
                /* code */
                break;
            case METRIC_TYPE::summary:
                update_summary();
                break;
            case METRIC_TYPE::status:
                status_map_[d->key] = d->str_value;
                break;
            default:
                break;
            }
        }
    });
}

void connect(const std::string& name) {
    struct sockaddr_un un;
    int sock_fd;
    char buffer[1024] = { 1, 2, 3 };
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, name.data());
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("Request socket failed\n");
        return;
    }
    if (connect(sock_fd, (struct sockaddr*)&un, sizeof(un)) < 0) {
        printf("connect socket failed\n");
        return;
    }
    send(sock_fd, buffer, 1024, 0);
}

Metric::~Metric() {
    std::string socket_name = socket_dir + app_tag_;
    is_running_ = false;
    if (t1_ && t1_->joinable()) {
        connect(socket_name);
        t1_->join();
    }
    if (t2_ && t2_->joinable()) {
        t2_->join();
    }
    unlink(socket_name.data());
}

void Metric::gen_and_write_output(int fd) {
    using namespace std;

    string buf;
    const string n = "\n";
    // const string prefix_string = app_tag_ + "_";
    const string prefix_string = "";

    buf = "======" + app_tag_ + " begin" + "======" + n;
    buf += app_tag_ + n;

    for (const auto& item : count_map_) {
        buf += prefix_string + item.first + "_count:" + to_string(item.second) + n;
    }

    for (const auto& item : guage_map_) {
        buf += prefix_string + item.first + "_guagae:" + to_string(item.second) + n;
    }

    lock_guard<mutex> lock(map_mutex);
    for (auto& item : summary_map_) {
        for (const auto& interval : summary_type_map_[item.first]) {
            int64_t sum = 0;
            for (auto index = 0; index < interval; index++) {
                int summary_index = summary_current_index_ - index;
                if (summary_index < 0) {
                    summary_index += summary_count;
                }
                sum += item.second[summary_index];
            }
            buf += prefix_string + item.first + "_" + std::to_string(interval) +
                   "_sum:" + std::to_string(sum) + n;
        }

        buf += prefix_string + item.first +
               "_total_sum:" + to_string(summary_total_map_[item.first]) + n;
    }

    for (const auto& item : status_map_) {
        buf += prefix_string + item.first + "_status:" + item.second + n;
    }

    buf += "=====" + app_tag_ + " end" + "=====" + n;

    // write(fd, buf.data(), buf.size());
    if (fd >= 0) {
        send(fd, buf.data(), buf.size(), MSG_NOSIGNAL);
    }

    // std::cout << buf << std::endl;
}

bool Metric::push_status(const std::string& key, const std::string& value) {
    auto data = std::make_shared<metric>();
    data->type = METRIC_TYPE::status;
    data->key = key;
    data->str_value = value;
    return push(data);
}

bool Metric::push_count(const std::string& key) {
    auto data = std::make_shared<metric>();
    data->type = METRIC_TYPE::count;
    data->key = key;
    return push(data);
}

bool Metric::push_guage(const std::string& key, int64_t value) {
    auto data = std::make_shared<metric>();
    data->type = METRIC_TYPE::gugae;
    data->key = key;
    data->int_value = value;
    return push(data);
}

bool Metric::push_summary(const std::string& key,
                          int64_t value,
                          const std::vector<int32_t>& interval) {
    auto data = std::make_shared<metric>();
    data->type = METRIC_TYPE::summary;
    data->key = key;
    data->int_value = value;
    data->interval = std::move(interval);
    return push(data);
}
#endif