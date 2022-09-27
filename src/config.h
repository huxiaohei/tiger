/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/22
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_CONFIG_H__
#define __TIGER_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <unordered_set>

#include "const.h"
#include "macro.h"

namespace tiger {

template <typename FromType, typename ToType>
class LexicalCast {
   public:
    ToType operator()(const FromType &v) {
        return boost::lexical_cast<ToType>(v);
    }
};

template <typename T>
class LexicalCast<std::string, std::vector<T>> {
   public:
    std::vector<T> operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        std::vector<T> v;
        std::stringstream ss;
        for (size_t i = 0; i < root.size(); ++i) {
            ss.str("");
            ss << root[i];
            v.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return v;
    }
};

template <typename T>
class LexicalCast<std::vector<T>, std::string> {
   public:
    std::string operator()(const std::vector<T> &v) {
        YAML::Node node;
        for (const auto &it : v) {
            node.push_back(LexicalCast<T, std::string>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T>
class LexicalCast<std::string, std::list<T>> {
   public:
    std::list<T> operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        std::list<T> v;
        std::stringstream ss;
        for (size_t i = 0; i < root.size(); ++i) {
            ss.str("");
            ss << root[i];
            v.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return v;
    }
};

template <typename T>
class LexicalCast<std::list<T>, std::string> {
   public:
    std::string operator()(const std::list<T> &v) {
        YAML::Node node;
        for (const auto &it : v) {
            node.push_back(LexicalCast<T, std::string>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T>
class LexicalCast<std::string, std::set<T>> {
   public:
    std::set<T> operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        std::set<T> v;
        std::stringstream ss;
        for (size_t i = 0; i < root.size(); ++i) {
            ss.str("");
            ss << root[i];
            v.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return v;
    }
};

template <typename T>
class LexicalCast<std::set<T>, std::string> {
   public:
    std::string operator()(const std::set<T> &v) {
        YAML::Node node;
        for (const auto &it : v) {
            node.push_back(LexicalCast<T, std::string>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T>
class LexicalCast<std::string, std::unordered_set<T>> {
   public:
    std::unordered_set<T> operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        std::unordered_set<T> v;
        std::stringstream ss;
        for (size_t i = 0; i < root.size(); ++i) {
            ss.str("");
            ss << root[i];
            v.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return v;
    }
};

template <typename T>
class LexicalCast<std::unordered_set<T>, std::string> {
   public:
    std::string operator()(const std::unordered_set<T> &v) {
        YAML::Node node;
        for (const auto &it : v) {
            node.push_back(LexicalCast<T, std::string>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T>
class LexicalCast<std::string, std::map<std::string, T>> {
   public:
    std::map<std::string, T> operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        std::map<std::string, T> v;
        std::stringstream ss;
        for (auto it = root.begin(); it != root.end(); ++it) {
            ss.str("");
            ss << it->second;
            v.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return v;
    }
};

template <typename T>
class LexicalCast<std::map<std::string, T>, std::string> {
   public:
    std::string operator()(const std::map<std::string, T> &v) {
        YAML::Node node;
        for (const auto &it : v) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
   public:
    std::unordered_map<std::string, T> operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        std::unordered_map<std::string, T> v;
        for (auto it = root.begin(); it != root.end(); ++it) {
            v.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(it->second.Scalar())));
        }
        return v;
    }
};

template <typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
   public:
    std::string operator()(const std::unordered_map<std::string, T> &v) {
        YAML::Node node;
        for (const auto &it : v) {
            node[it.first] = LexicalCast<T, std::string>()(it.second);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

class ConfValBase {
   private:
    std::string m_name;
    std::string m_desc;

   public:
    typedef std::shared_ptr<ConfValBase> ptr;

    ConfValBase(const std::string &name, const std::string &desc = "")
        : m_name(name), m_desc(desc) {}
    virtual ~ConfValBase() {}

    const std::string &name() { return m_name; }
    const std::string &desc() { return m_desc; }

    virtual std::string to_string() = 0;
    virtual bool from_string(const std::string &str) = 0;
};

template <typename T>
class ConfigVar : public ConfValBase {
   private:
    T m_val;

   public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string &name, const T &def, const std::string &desc = "")
        : ConfValBase(name, desc), m_val(def) {}
    ~ConfigVar(){};

    const T &val() { return m_val; }
    void set_val(const T &v) { m_val = v; }

   public:
    std::string to_string() override {
        try {
            return LexicalCast<T, std::string>()(val());
        } catch (const std::exception &e) {
            TIGER_LOG_E(SYSTEM_LOG) << "TO STRING ERROR: " << e.what();
            return "";
        }
    }

    bool from_string(const std::string &str) override {
        try {
            set_val(LexicalCast<std::string, T>()(str));
            return true;
        } catch (const std::exception &e) {
            TIGER_LOG_E(SYSTEM_LOG) << "FROM STRING ERROR: " << e.what();
            return false;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const ConfigVar<T>::ptr conf) {
        os << "\n[\n" << conf->name() << "," << conf->desc() << ",\n" << conf->to_string() << "\n]";
        return os;
    }
};

class Config {
   public:
    template <typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name, const T &def, const std::string &desc = "") {
        auto it = Lookup<T>(name);
        if (it) return it;
        if (!IsValidName(name)) {
            TIGER_LOG_E(SYSTEM_LOG) << "CONFIG NAME ERROR: " << name;
            throw std::invalid_argument(name);
        }
        it = std::make_shared<ConfigVar<T>>(name, def, desc);
        auto &datas = S_DATAS();
        datas[name] = it;
        return it;
    }

    template <typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name) {
        auto it = S_DATAS().find(name);
        if (it == S_DATAS().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static ConfValBase::ptr LookupBase(const std::string &name);
    static void LoadFromYmal(const std::string &name, const std::string &ymal_path);

   private:
    static std::map<std::string, ConfValBase::ptr> &S_DATAS() {
        static std::map<std::string, ConfValBase::ptr> s_datas;
        return s_datas;
    }
};

}  // namespace tiger

#endif