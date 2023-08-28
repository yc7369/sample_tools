#ifndef __SDS_UTIL__
#define __SDS_UTIL__

#pragma once

#include <sys/stat.h>
//#include <netinet/in.h>
#include <sys/timeb.h>
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <memory>
#include <cassert>
#include <cctype>
#ifndef _MSC_VER
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sched.h>
#else
#include <io.h>
#include <direct.h>
#define access(path, mode) _access(path, mode)
#define mkdir(dir, mode) _mkdir(dir)
#endif

namespace util {

#ifdef _MSC_VER
#ifndef popen
#define popen _popen
#define pclose _pclose

#endif  // ! popen

#else
#endif

/**
 * @brief 解析诸如 ssl://172.24.13.81:9001, tcp://172.24.13.81:9001 ,172.24.13.81:9001 的url
 *
 * @param url 需要解析的url
 * @param is_ssl 如果有 ssl://标记 则认为是ssl，其他为非ssl
 * @param host url中的host
 * @param port url 中的port
 * @return true 解析成功
 * @return false 解析失败
 */
inline bool ParseUrl(const std::string& url, bool& is_ssl, std::string& host, std::string& port) {

    if (url.empty()) {
        return false;
    }

    if (url.find("ssl://") != url.npos) {
        is_ssl = true;
    } else {
        is_ssl = false;
    }
    try {
        auto pos = url.find_last_of("/");
        auto pos2 = url.find_last_of(":");
        host = url.substr(pos + 1, pos2 - pos - 1);
        port = url.substr(pos2 + 1);
    } catch (const std::exception& e) {
        // std::cerr << e.what() << '\n';
        return false;
    }
    return true;
}

/**
 * @brief 将小写字母转为大写，内部函数，不检查参数
 *
 * @param data
 * @return const std::string
 */
inline const std::string toUp(const char* data) {
    // if (data == nullptr) {
    //     return "";
    // }
    std::string d(data);
    for (size_t i = 0; i < d.size(); i++) {
        if (d.at(i) >= 'a' && d.at(i) <= 'z') {
            d.at(i) = d.at(i) - ('a' - 'A');
        }
    }
    return d;
    // c++17 或者之后
    //  moving a local object in a return statement prevents copy elision [-Wpessimizing-move]
    // return std::move(d);
}

inline int system(std::string cmd, std::vector<std::string>& result) {
    int ret = 0;
    FILE* fp = NULL;
    const size_t BUF_SIZE = 1024;
    char buf[BUF_SIZE] = { 0 };
    cmd += " 2>&1";

    do {
        fp = popen(cmd.data(), "r");
        if (NULL == fp) {
            ret = -1;
            break;
        }
        while (NULL != fgets(buf, BUF_SIZE, fp)) {
            std::string data = std::string(buf);
            if (data.size() > 0) {
                result.push_back({ data.data(), data.size() - 1 });
            }
            memset(buf, 0, BUF_SIZE);
        }
        pclose(fp);
    } while (0);

    fp = NULL;
    return ret;
}

inline bool get_master_host_from_etcd(const std::string& endpoints,
                                      const std::string& key,
                                      std::vector<std::pair<std::string, std::string>>& host) {
    std::vector<std::string> result;

    std::string cmd;

    cmd = "etcdctl --endpoints=" + endpoints + " get --prefix " + key;

    if (0 != system(cmd, result)) {
        return false;
    }

    if (result.size() < 2) {
        return false;
    }

    std::string tmp_str;

    //奇数 key，偶数val
    for (size_t i = 1; i < result.size(); i = i + 2) {
        tmp_str = result[i];
        auto pos = result[i].find_last_of("/");

        if (pos != std::string::npos) {
            tmp_str = result[i].substr(pos + 1);
        }
        pos = tmp_str.find(":");
        if (pos != std::string::npos) {
            host.emplace_back(tmp_str.substr(0, pos), tmp_str.substr(pos + 1));
        }
    }

    return true;
}

inline bool
set_key_to_etcd(const std::string& endpoints, const std::string& key, const std::string& val) {
    std::string cmd;

    cmd = "etcdctl --endpoints=" + endpoints + " put " + key + " " + val;

    std::vector<std::string> result;

    if (0 != system(cmd, result)) {
        return false;
    }

    if (result.size() && result[0] == "OK") {
        return true;
    }

    return false;
}

// trim from start
inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); }));
    return s;
}

// trim from end
inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(),
            s.end());
    return s;
}

// trim from both ends
inline std::string& trim(std::string& s) {
    return ltrim(rtrim(s));
}

inline int split(const std::string& s, const std::string& delim, std::vector<std::string>* ret) {
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos) {
        ret->push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0) {
        ret->push_back(s.substr(last, index - last));
    }
    return 0;
}

inline bool get_full_endpoints_etcd(const std::string& endpoints, std::string& full_endpoints) {
    std::string cmd;
    cmd = "etcdctl --endpoints=" + endpoints + " member list";
    std::vector<std::string> result;

    if (0 != system(cmd, result)) {
        return false;
    }
    // line :
    // a4250f1740f7c7a, started, node1, http://172.24.13.82:2380, http://172.24.13.82:2379, false
    for (auto line : result) {
        std::vector<std::string> tmp;
        if (0 == split(line, ",", &tmp)) {
            if (tmp.size() != 6) {
                return false;
            }
            if (full_endpoints.size()) {
                full_endpoints += ",";
            }
            full_endpoints += trim(tmp[4]);  //取2379这个就行,去除前后空格
        }
    }
    return full_endpoints.size();
}

inline int mkdir_p(const char* path) {
    size_t len = ::strlen(path);
    char* tmp = (char*)::malloc(len + 1);
    char* pos = tmp;
    const char* cur = path;
    while (*cur++ != '\0') {
        *pos++ = *(cur - 1);
        *pos = 0;
        if (*cur == '/' || *cur == '\\' || *cur == '\0') {
            if (::access(tmp, 0) != 0)
                ::mkdir(tmp, 0755);
        }
    }
    free(tmp);
    return 0;
}

inline int get_clean_path(const std::string& path, std::string* clean_path) {
    std::vector<std::string> vec;
    split(path, "/", &vec);
    auto it = vec.begin();
    while (it != vec.end()) {
        if (!it->compare(".")) {
            it = vec.erase(it);
        } else if (!it->compare("..")) {
            it = vec.erase(it - 1, it);
        } else
            ++it;
    }

    for (it = vec.begin(); it != vec.end(); ++it) {
        if (it != vec.begin())
            *clean_path += "/";
        *clean_path += *it;
    }
    return 0;
}

inline int get_full_path(const std::string& dir, const std::string& path, std::string* full_path) {
    *full_path = dir;
    if (!full_path->empty())
        *full_path += "/";
    *full_path += path;
    return 0;
}

#ifndef _MSC_VER

//设置当前进程为实时进程
//必须以root权限启动才能设置成功
inline void set_rt_process() {
    pid_t pid = getpid();
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);  // 也可用SCHED_RR
    sched_setscheduler(pid, SCHED_RR, &param);                  // 设置当前进程
}

//设置当前线程为实时线程
//必须以root权限启动才能设置成功
inline void set_rt_thread() {
    pid_t pid = getpid();
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);  // 也可用SCHED_RR
    sched_setscheduler(pid, SCHED_RR, &param);                  //
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
}

/*******************************************************************
File Name: include/util.h
Description: 设置当前线程绑定到指定CPU ID
Calls: pthread_setaffinity_np pthread_self
Table Updated: 无
Input:
    cpud_id： 指定的CPU ID
Output:
    无
Return:
    true: 绑定成功 false：绑定失败
Others: 无
History:
    无
********************************************************************/
inline bool bind_current_thread(int cpud_id) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpud_id, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        perror("bindCurrentThreadWith");
        return false;
    }
    return true;
}
#endif  // !_MSC_VER

template <class T>
std::vector<T> input_vec(std::string arg, char sep = ',') {
    using namespace std;
    istringstream iss;
    replace(arg.begin(), arg.end(), sep, ' ');
    iss.str(arg);
    T value;
    vector<T> params;
    while (iss >> value) {
        params.push_back(value);
    }
    return params;
}

template <class T>
std::set<T> input_set(std::string arg, char sep = ',') {
    using namespace std;
    istringstream iss;
    replace(arg.begin(), arg.end(), sep, ' ');
    iss.str(arg);
    T value;
    set<T> params;
    while (iss >> value) {
        params.insert(value);
    }
    return params;
}

template <class KT, class VT>
std::map<KT, VT> input_map(std::string arg, char sep_entry = ',', char sep_kv = '=') {
    using namespace std;
    istringstream iss;
    replace(arg.begin(), arg.end(), sep_entry, ' ');
    replace(arg.begin(), arg.end(), sep_kv, ' ');
    iss.str(arg);
    KT key;
    VT value;
    map<KT, VT> params;
    while ((iss >> key) && (iss >> value)) {
        params.insert({ key, value });
    }
    return params;
}

#ifndef _MSC_VER

inline std::vector<std::string> get_files(const std::string& cate_dir, bool getDir) {
    using namespace std;

    vector<string> files;  //存放文件名

    DIR* dir;
    struct dirent* ptr;
    char base[1024];

    if ((dir = opendir(cate_dir.c_str())) == nullptr) {
        sprintf(base, "Open dir %s error...\n", cate_dir.c_str());
        perror(base);
        return files;
    }

    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 ||
            strcmp(ptr->d_name, "..") == 0) {  /// current dir OR parrent dir
            continue;
        }
        if (((ptr->d_type == 8) && (!getDir)) ||  /// file
            ((ptr->d_type == 4) && getDir)) {     /// dir
            // printf("d_name:%s/%s\n",basePath,ptr->d_name);
            files.push_back(ptr->d_name);
        }
        /*/
        else if(ptr->d_type == 10)    ///link file
        //printf("d_name:%s/%s\n",basePath,ptr->d_name);
        continue;
        else if(ptr->d_type == 4)    ///dir
        {
        files.push_back(ptr->d_name);

        //memset(base,'\0',sizeof(base));
        //strcpy(base,basePath);
        //strcat(base,"/");
        //strcat(base,ptr->d_nSame);
        //readFileList(base);

        }
        /*/
    }
    closedir(dir);

    //排序，按从小到大排序
    sort(files.begin(), files.end());
    return files;
}
#endif
inline int CreateDir(const char* pszDir) {
    int i = 0;
    int iLen = strlen(pszDir);
    //在末尾加/
    char dir_path[100];
    // if (pszDir[iLen - 1] != '\\' && pszDir[iLen - 1] != '/')
    if (pszDir[iLen - 1] != '/') {
        // dir_path[iLen] = '/';
        dir_path[iLen] = '/';
        dir_path[iLen + 1] = '\0';
    } else
        dir_path[iLen] = '\0';
    strncpy(dir_path, pszDir, iLen);

    int len = strlen(dir_path);
    for (i = 0; i < len; i++) {
        // if (dir_path[i] == '/' && i > 0)
        if (dir_path[i] == '/' && i > 0) {
            dir_path[i] = '\0';  //一级一级建目录
            if (access(dir_path, 0) < 0) {
                if (mkdir(dir_path, 0755) < 0) {
                    // printf("mkdir=%s:msg=%s\n", dir_path, strerror_s(errno));
                    return -1;
                }
            }
            // dir_path[i] = '/';
            dir_path[i] = '/';
        }
    }
    return 0;
}

#ifndef _MSC_VER

typedef uint64_t tcp_id_t;
// TCP连接的ID由 远程ip(64-33bit), 远程port(32-17bit), 本地port(16-1) 三部分组成
inline tcp_id_t get_tcp_id(int connfd) {
    struct sockaddr_in sock_addr;
    socklen_t peerLen = sizeof(sock_addr);
    getpeername(connfd, (struct sockaddr*)&sock_addr, &peerLen);
    tcp_id_t tcp_id = sock_addr.sin_addr.s_addr;
    tcp_id <<= 32;
    tcp_id += sock_addr.sin_port << 16;
    getsockname(connfd, (struct sockaddr*)&sock_addr, &peerLen);
    tcp_id += sock_addr.sin_port;
    return tcp_id;
}
// TCP连接的名字:
inline std::string get_tcp_name(tcp_id_t tcp_id) {
    struct in_addr sin_addr;
    sin_addr.s_addr = tcp_id >> 32;
    int peer_port = (tcp_id << 32) >> 48;
    int sock_port = (tcp_id << 48) >> 48;

    char tcp_name[INET_ADDRSTRLEN + 1 + 5 + 1 + 5];
    inet_ntop(AF_INET, &sin_addr, tcp_name, sizeof(tcp_name));
    auto p = tcp_name;
    while (*p != '\0')
        ++p;
    sprintf(p, ":%d-%d", peer_port, sock_port);
    return tcp_name;
}
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName)    \
    TypeName(const TypeName&) = delete;       \
    void operator=(const TypeName&) = delete; \
    TypeName(const TypeName&&) = delete;      \
    void operator=(const TypeName&&) = delete

}  // namespace util

#ifndef DEFINE_SHARED_PTR
#define DEFINE_SHARED_PTR(TypeName) typedef std::shared_ptr<TypeName> TypeName##Ptr
#endif

#endif
