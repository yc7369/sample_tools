#ifndef METRIC_H
#define METRIC_H

#ifndef _MSC_VER

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include "concurrentqueue.h"
#include "mybase/timerman.h"

class Metric {

    enum class METRIC_TYPE { count = 0, gugae = 1, histogram = 2, summary = 3, status = 4 };

    struct metric {
        METRIC_TYPE type;
        int64_t times;  // 秒时间戳
        std::string key;
        std::vector<int32_t> interval;  // 数据输出间隔，秒
        int64_t int_value;
        std::string str_value;
    };

public:
    static Metric* inst() {
        static Metric m;
        return &m;
    }

    void run(const std::string& app_tag);

public:
    /**
     * @brief 每push一次，递增一次
     *
     * @param key 键
     * @return true 成功
     * @return false 失败
     */
    bool push_count(const std::string& key);
    /**
     * @brief 随意设置数字值
     *
     * @param key 键
     * @return true 成功
     * @return false 失败
     */
    bool push_guage(const std::string& key, int64_t value);
    // void push_histogram(const std::string& key, int64_t value);

    /**
     * @brief 统计数据接口
     *
     * @param key 键
     * @param value 值
     * @param interval 统计输出间隔  如{1,2,5,9,100} 共输出6个
     * @return true 成功
     * @return false 失败
     */
    bool push_summary(const std::string& key, int64_t value, const std::vector<int32_t>& interval);

    /**
     * @brief 统计字符串值 如启动成功
     *
     * @param key 键
     * @param value 值
     * @return true 成功
     * @return false 失败
     */
    bool push_status(const std::string& key, const std::string& value);

private:
    inline bool push(std::shared_ptr<metric> data) {
        // return true;
        data->times = mybase::time::unixTimeNowMs() / 1000;
        return is_running_ ? q_.enqueue(data) : false;
    }
    void gen_and_write_output(int fd);
    Metric();
    ~Metric();

private:
    std::string app_tag_;

    std::mutex map_mutex;  // 所有的map mutext

    // 只能加不能减少,一般用来记录重连次数，登录次数等状态
    std::unordered_map<std::string, uint64_t> count_map_;

    // 可任意变化的指标,即可增可减
    std::unordered_map<std::string, uint64_t> guage_map_;

    // Histogram 好像没什么应用场景 使用某些量化指标的平均值
    // key- v[time,value]
    std::unordered_map<std::string, std::vector<std::pair<int64_t, uint64_t>>> histogram_map_;

    // key->value[100 个数组] 每个数组保存当前这秒内地流量
    static const int64_t summary_count = 1000;
    std::unordered_map<std::string, std::array<int64_t, summary_count>> summary_map_;
    uint16_t summary_current_index_;
    std::unordered_map<std::string, uint64_t> summary_total_map_;
    // summary_map_ 的输出类型，时间间隔多少,vector 多大就输出几个
    std::unordered_map<std::string, std::vector<int32_t>> summary_type_map_;

    // 保存最新的状态，这是字符串值
    std::unordered_map<std::string, std::string> status_map_;

    std::atomic_bool is_running_ = { false };

    moodycamel::ConcurrentQueue<std::shared_ptr<metric>> q_;
    std::shared_ptr<std::thread> t1_;
    std::shared_ptr<std::thread> t2_;
};

#define METRIC_COUNT(key) Metric::inst()->push_count(key)
#define METRIC_GUAGE(key, value) Metric::inst()->push_guage(key, value)
#define METRIC_STATUS(key, value) Metric::inst()->push_status(key, value)
inline bool
METRIC_SUMMARY(const std::string& key, int64_t value, const std::vector<int32_t>& interval) {
    return Metric::inst()->push_summary(key, value, interval);
}

#else

#define METRIC_COUNT(key)
#define METRIC_GUAGE(key, value)
#define METRIC_STATUS(key, value)
#define METRIC_SUMMARY(key, value, V)

#endif

#endif