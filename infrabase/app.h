#ifndef __LINKER_CONFIG_H__
#define __LINKER_CONFIG_H__

#pragma once

#include <iostream>
#include <iomanip>
#include "mjson/mjson.h"
#include "infrabase/string_assist.h"
#include "infrabase/cmdline.h"
#include "infrabase/util.h"
#include "zlog/ztp_log.h"
#include "linker/linkerapi.h"

using namespace linker;
namespace sdsx {
#define SDS_SECTION "sds"

template <typename OptType = linker::BaseOption>
class Config {
public:
    typedef std::function<bool(MJson&, OptType&)> OptionParser;

public:
    Config() {
        _options.log_level = 0;
        _options.log_dir = "./log/";
    }

    OptType _options;
    LinkerConfig _linker_cfg;

    void debugPrint() const {
        std::cout << "configs:" << std::endl;
        std::cout << "------------------------------" << std::endl;
#define PRINT_FILED(x, f) \
    std::cout << std::right << std::setw(22) << #f << "  :  " << std::left << x.f << std::endl;
#define PRINT_VECTOR_FILED(x, f)               \
    std::cout << #f << ":\n----" << std::endl; \
    for (const auto& s : x.f) {                \
        std::cout << s << std::endl;           \
    }

        auto& v = _options;
        PRINT_FILED(v, log_level);
        PRINT_FILED(v, log_dir);
        PRINT_FILED(v, log_code);

        auto& v1 = _linker_cfg;

        PRINT_FILED(v1, agent_work_cpu_id);  //;
        PRINT_FILED(v1, agent_high_perfomance);
        PRINT_FILED(v1, shm_id);                // = 0;
        PRINT_FILED(v1, etcd_endpoints);        //;
        PRINT_VECTOR_FILED(v1, listen_urls);    //;
        PRINT_FILED(v1, shm_write_close_time);  // = "16,20";
        PRINT_FILED(v1, shm_write_base_id);     // = "110";
        PRINT_FILED(v1, shm_write_suffix);      // = "_2.0";
        PRINT_VECTOR_FILED(v1, msg_handlers);   // = 0;

        std::cout << " ztp_sub_url:" << std::endl;
        for (auto s : v1.tcp_sub_urls) {
            for (auto s2 : s) {
                std::cout << " [" << s2 << "]" << std::ends;
            }
            std::cout << "\n";
        }
#undef PRINT_FILED
#undef PRINT_VECTOR_FILED
        std::cout << "------------------------------" << std::endl;
    }

    bool Load(const std::string& file = "./config.json", OptionParser cfg_parser = nullptr) {
        _jsonfile = file;
        std::cout << "loading config file:" << _jsonfile << std::endl;
        _linker_cfg.config_file = _jsonfile;
        MJson json(_jsonfile);
        if (json.ParseError() != 0) {
            std::cout << "Can't load " << _jsonfile << std::endl;
            return false;
        }

        if (parseOptions(json)) {
            if (cfg_parser && !cfg_parser(json, _options)) {
                return false;
            }

            debugPrint();
            return true;
        }
        return false;
    }

protected:
    virtual bool parseOptions(MJson& json) {
        // logs
        _options.log_level = json.GetInteger(SDS_SECTION, "log_level", 1);
        _options.log_dir = json.Get(SDS_SECTION, "log_dir", "./top");
        _options.log_code = json.Get(SDS_SECTION, "log_code", "600446.sh");
        _linker_cfg.agent_work_cpu_id = json.GetInteger(SDS_SECTION, "agent_work_cpu_id", -1);
        _linker_cfg.agent_high_perfomance =
            json.GetBoolean(SDS_SECTION, "agent_high_perfomance", true);

        // ztp
        json.GetArray(SDS_SECTION, "ztp_pub_url", _linker_cfg.listen_urls);

        std::vector<std::vector<std::string>> tmp_vecs;

        if (json.GetArray(SDS_SECTION, "ztp_sub_url", tmp_vecs)) {
            for (auto i : tmp_vecs) {
                if (i.size() < 2) {
                    std::cerr << "get services error" << std::endl;
                    exit(-1);
                }
                ServiceConfig c{};
                c.name = i[0];
                c.url = i[1];
                if (i.size() == 3) {
                    c.hostkey = i[2];
                }
                _linker_cfg.services.push_back(c);
            }
        }

        std::vector<std::vector<std::string>> vecs;
        if (json.GetArray(SDS_SECTION, "compress_time", vecs)) {
            for (auto i : vecs) {
                if (i.size() < 2) {
                    std::cerr << "get compress_time error" << std::endl;
                    exit(-1);
                }
                CompressConfig c{};
                c.time_start = atoi(i[0].c_str());
                c.time_end = atoi(i[1].c_str());
                _linker_cfg.compress_time.push_back(c);
            }
        }

        std::vector<std::string> endpoints;

        json.GetArray(SDS_SECTION, "endpoints", endpoints);

        //若当前没填 endpoints，则取 etcd域中的 endpionts，简化配置
        if (endpoints.size() == 0) {
            json.GetArray("etcd", "endpoints", endpoints);
        }

        for (auto i : endpoints) {
            if (_linker_cfg.etcd_endpoints.size()) {
                _linker_cfg.etcd_endpoints += ",";
            }
            _linker_cfg.etcd_endpoints += i;
        }

        // shm
        _linker_cfg.shm_write_close_time = json.Get(SDS_SECTION, "shm_write_close_time", "16,20");
        _linker_cfg.shm_write_base_id = json.Get(SDS_SECTION, "shm_write_base_id", "110");
        _linker_cfg.shm_write_suffix = json.Get(SDS_SECTION, "shm_write_suffix", "_2.0");
        _linker_cfg.is_open_index = json.GetInteger(SDS_SECTION, "is_open_index", 0);

        _linker_cfg.shm_id = json.GetInteger(SDS_SECTION, "shm_id", 9901);

        _linker_cfg.listen_etcd_url = json.Get(SDS_SECTION, "bind_etcd_key", "");

        auto handlers = json.Get(SDS_SECTION, "msg_handlers", "");
        if (!handlers.empty()) {
            util::StringAssist::split(handlers, ",", _linker_cfg.msg_handlers);
        }
        json.GetArray(SDS_SECTION, "tcp_sub_topic", _linker_cfg.tcp_sub_topic);

        std::vector<std::string> input_types;

        // 输入类型  ztp,hare,
        json.GetArray(SDS_SECTION, "input_types", input_types);
        for (auto i : input_types) {
            _linker_cfg.input_types.insert(i);
        }

        // 默认输入类型为 ztp
        if (_linker_cfg.input_types.size() == 0) {
            _linker_cfg.input_types.insert("ztp");
        }
        std::vector<std::string> output_types;

        // 输出类型  ztp,hare,
        json.GetArray(SDS_SECTION, "output_types", output_types);
        for (auto i : output_types) {
            _linker_cfg.output_types.insert(i);
        }

        // 默认输出类型为 ztp
        if (_linker_cfg.output_types.size() == 0) {
            _linker_cfg.output_types.insert("ztp");
        }

        _linker_cfg.use_hare = json.GetBoolean(SDS_SECTION, "use_hare", false);
        _linker_cfg.hare_conf_dir = json.Get(SDS_SECTION, "hare_conf_dir", "");

        return true;
    }

private:
    std::string _jsonfile;
};
// typedef Config<BaseOption> BaseConfig;

// Application Helper
// 用于命令行程序的基本初始化
template <typename AppOption = BaseOption>
class AppInfos {
    typedef Config<AppOption> ConfigType;

public:
    AppInfos(const std::string& app_name, const std::string& version)
        : version_(version), app_name_(app_name) {
        ap_.add("version", 'v', "print version");
        ap_.add<std::string>("conf", 'c', "config file path", false, "./config.json");
        cfg_ = std::make_shared<ConfigType>();
    }

    // ret 含义：
    // > 0 , 执行正常，需要继续运行
    // = 0, 正常，但是不应该继续执行，比如要求打印version
    // < 0, 错误，不能继续执行
    // without_config_file: true 不需要配置文件
    int init(int argc,
             char* argv[],
             typename ConfigType::OptionParser cfg_parser = nullptr,
             bool without_config_file = false) {
        ap_.parse_check(argc, argv);
        if (ap_.exist("version")) {
            printVersion();
            exit(0);
            return 0;
        }

        if (!without_config_file) {
            if (!cfg_->Load(ap_.get<std::string>("conf"), cfg_parser)) {
                std::cerr << "FATAL: config load failed! exiting ..." << std::endl;
                return -1;
            }
        }

        auto& opts = cfg_->_options;
        if (0 != util::CreateDir(opts.log_dir.c_str())) {
            std::cerr << "创建日志目录失败:" << opts.log_dir << std::endl;
            return -2;
        }

        ztp::setup_log_system(
            opts.log_dir, BASE_LOGGER_NAME, opts.log_level, BASE_LOGGER_NAME, false);

        return 1;
    }

    std::shared_ptr<ConfigType> config() {
        return cfg_;
    }

    cmdline::parser& cmd() {
        return ap_;
    }

    const std::string& version() const {
        return version_;
    }

    const std::string& name() const {
        return app_name_;
    }

    std::string app_tag() const {
        LinkerConfig& cfg = cfg_->_linker_cfg;

        std::string suffix;

        if (cfg.listen_urls.size() > 1) {
            suffix = cfg.listen_urls[1];
        } else {
            for (auto i : cfg.output_types) {
                suffix += i;
            }
        }

        return app_name_ + "_" + suffix;
    }

    void printVersion() const {
        std::cout << version_ << std::endl;
    }

    // func: 返回0 正常继续执行，返回非0，程序将退出
    int run(std::function<int(LinkerContext* ctx)> func = nullptr) {

        if (!cfg_) {
            return -1;
        }
        LinkerContext* ctx = nullptr;
        if (!init_linker_context(cfg_->_linker_cfg, ctx)) {
            return -2;
        }

        // 外部初始化机会
        if (func) {
            int ret = func(ctx);
            if (ret != 0) {
                return ret;
            }
        }

        // 运行系统
        run_linker_service();
        return 0;
    }

private:
    std::string version_;
    std::string app_name_;
    cmdline::parser ap_;
    std::shared_ptr<ConfigType> cfg_ = nullptr;
};

}  // namespace sdsx
#endif
