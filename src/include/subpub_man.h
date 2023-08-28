#pragma once

#include <unordered_map>
#include <vector>
#include <set>
#include <memory>
#include <algorithm>
#include <sstream>
#include <mutex>

namespace helper {

using TopicType = int64_t;
template <typename SuberType, typename KeyType>
class Topics {
public:
    Topics(TopicType topic) : topic_(topic) {}

    std::string dumpSubers() const {
        std::ostringstream oss;
        oss << "--------------------------------" << std::endl;
        // oss << "Key Suber:" << topic_ << std::endl;
        for (auto i : subers_) {
            std::string ks;
            for (auto& j : i.second) {
                if (ks.size()) {
                    ks += ", ";
                }
                ks += std::to_string(j);
            }
            oss << "  <Key, Suber>:[" << i.first << "] <==> [" << ks << "]" << std::endl;
        }
        // oss << "Topic Suber:" << topic_ << std::endl;
        if (!topic_subers_.empty()) {
            std::string ks;
            for (auto i : topic_subers_) {
                if (ks.size()) {
                    ks += ", ";
                }
                ks += std::to_string(i);
            }
            oss << "  <Topic, Suber>:[" << topic_ << "] <==> [" << ks << "]" << std::endl;
        }
        oss << "--------------------------------" << std::endl;
        return oss.str();
    }

    void addTopicSuber(const SuberType& st) {
        std::lock_guard<std::mutex> lock(m_);

        topic_subers_.insert(st);
    }

    void addSubscriber(const KeyType& key, const SuberType& st) {
        std::lock_guard<std::mutex> lock(m_);

        auto it = subers_.find(key);
        if (it != subers_.end()) {
            it->second.insert(st);
        } else {
            subers_.insert(std::make_pair(key, std::set<SuberType>({ st })));
        }
    }

    void removeSubscriber(const KeyType& key, const SuberType& st) {
        std::lock_guard<std::mutex> lock(m_);

        auto it = subers_.find(key);
        if (it != subers_.end()) {
            it->second.erase(st);
        }
    }

    void removeSubscriber(const SuberType& st) {
        std::lock_guard<std::mutex> lock(m_);

        topic_subers_.erase(st);
        for (auto& i : subers_) {
            i.second.erase(st);
        }
    }

    const std::set<SuberType> subers(const KeyType& key) {
        std::lock_guard<std::mutex> lock(m_);
        std::set<SuberType> _ts;

        if (subers_.find(key) == subers_.end()) {
            return topic_subers_;
        }

        if (topic_subers_.empty()) {
            return subers_[key];
        }

        _ts = subers_[key];
        for (auto t : topic_subers_) {
            _ts.insert(t);
        }
        return _ts;
    }

private:
    std::mutex m_;
    TopicType topic_;
    // subscribers: <key, suber>
    std::unordered_map<KeyType, std::set<SuberType>> subers_;
    // 订阅整个topic的列表
    std::set<SuberType> topic_subers_;
};

/*! \class SubManager 实现一个简单的订阅发布系统
 *      目前的需求是：仅支持单key订阅发布即可.
 *      注意：非线程安全类
 */
template <typename SuberType, typename KeyType>
class SubManager {
    typedef Topics<SuberType, KeyType> TopicObject;
    typedef std::shared_ptr<TopicObject> TopicObjectPtr;

public:
    // 订阅
    void Subscribe(TopicType topic, const KeyType& key, const SuberType& st) {
        if (auto t = get(topic, true)) {
            t->addSubscriber(key, st);
        }
    }

    // 订阅topic下所有的key
    void SubscribeTopic(TopicType topic, const SuberType& st) {
        if (auto t = get(topic, true)) {
            t->addTopicSuber(st);
        }
    }

    // 取消订阅topic下所有的key
    void UnsubscribeTopic(TopicType topic, const SuberType& st) {
        if (auto t = get(topic, true)) {
            t->removeSubscriber(st);
        }
    }

    // 取消object下某topic/key 订阅
    void Unsubscribe(TopicType topic, const KeyType& key, const SuberType& st) {
        if (auto t = get(topic, false)) {
            t->removeSubscriber(key, st);
        }
    }

    // 取消某object下的所有订阅
    void Unsubscribe(const SuberType& st) {
        for (auto i : subers_) {
            i.second->removeSubscriber(st);
        }
    }

    const std::set<SuberType> subers(TopicType topic, const KeyType& key) {
        static std::set<SuberType> _empty;
        if (auto t = get(topic)) {
            return t->subers(key);
        }
        return _empty;
    }

    // 打印订阅列表
    std::string dumpSubers() {
        std::ostringstream oss;
        for (auto i : subers_) {
            oss << i.second->dumpSubers();
        }
        return oss.str();
    }

protected:
    TopicObjectPtr get(TopicType topic, bool create = false) {
        auto i = subers_.find(topic);
        if (i != subers_.end()) {
            return i->second;
        }

        if (create) {
            auto p = std::make_shared<TopicObject>(topic);
            subers_[topic] = p;
            return p;
        }
        return nullptr;
    }

private:
    // <topic, topicman>
    std::unordered_map<TopicType, TopicObjectPtr> subers_;
};
}  // namespace helper
