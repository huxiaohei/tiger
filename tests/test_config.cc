/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

const std::string LOG_NAME = "TEST_CONF";

void test_load_from_path() {
    tiger::ConfigVar<int>::ptr int_cfg = tiger::Config::Lookup("test.int", (int)8080, "int");
    tiger::ConfigVar<float>::ptr float_cfg = tiger::Config::Lookup("test.float", (float)3.14, "float");
    tiger::ConfigVar<std::string>::ptr str_cfg = tiger::Config::Lookup("test.str", (std::string) "Hello", "string");
    tiger::ConfigVar<std::vector<std::string>>::ptr vector_cfg = tiger::Config::Lookup("test.vector", (std::vector<std::string>){"Hello"}, "vector");
    tiger::ConfigVar<std::list<std::string>>::ptr list_cfg = tiger::Config::Lookup("test.list", (std::list<std::string>){}, "list");
    tiger::ConfigVar<std::set<std::string>>::ptr set_cfg = tiger::Config::Lookup("test.set", (std::set<std::string>){}, "set");
    tiger::ConfigVar<std::unordered_set<std::string>>::ptr unordered_set_cfg = tiger::Config::Lookup("test.unordered_set", (std::unordered_set<std::string>){}, "unordered_set");
    tiger::ConfigVar<std::map<std::string, std::string>>::ptr map_cfg = tiger::Config::Lookup("test.map", (std::map<std::string, std::string>){}, "map");
    tiger::ConfigVar<std::unordered_map<std::string, std::string>>::ptr unordered_map_cfg = tiger::Config::Lookup("test.unordered_map", (std::unordered_map<std::string, std::string>){}, "unordered_map");
    tiger::Config::LoadFromYmal("test", "../tests/test_config.yml");
    TIGER_LOG_D(LOG_NAME) << "int_cfg:" << int_cfg;
    TIGER_LOG_D(LOG_NAME) << "float_cfg:" << float_cfg;
    TIGER_LOG_D(LOG_NAME) << "str_cfg:" << str_cfg;
    TIGER_LOG_D(LOG_NAME) << "vector_cfg:" << vector_cfg;
    TIGER_LOG_D(LOG_NAME) << "list_cfg:" << list_cfg;
    TIGER_LOG_D(LOG_NAME) << "set_cfg:" << set_cfg;
    TIGER_LOG_D(LOG_NAME) << "unordered_set_cfg:" << unordered_set_cfg;
    TIGER_LOG_D(LOG_NAME) << "map_cfg:" << map_cfg;
    TIGER_LOG_D(LOG_NAME) << "unordered_map:" << unordered_map_cfg;
}

class TestObject {
   private:
    std::string m_name;
    int m_val;

   public:
    typedef std::shared_ptr<TestObject> ptr;

    TestObject(const std::string &name, int val)
        : m_name(name), m_val(val) {
    }

    const std::string &name() { return m_name; }
    const int val() { return m_val; }

   public:
    std::string to_string() {
        std::stringstream ss;
        ss << "[TestObject]\n"
           << "\t" << m_name << "\n"
           << "\t" << m_val;
        return ss.str();
    };
};

template <>
class tiger::LexicalCast<std::string, TestObject::ptr> {
   public:
    TestObject::ptr operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        TestObject::ptr v = std::make_shared<TestObject>(root["name"].as<std::string>(), root["val"].as<int>());
        return v;
    }
};

template <>
class tiger::LexicalCast<TestObject::ptr, std::string> {
   public:
    std::string operator()(const TestObject::ptr v) {
        YAML::Node node;
        node["name"] = v->name();
        node["val"] = v->val();
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

void test_object() {
    auto def_obj = std::make_shared<TestObject>("Hello", 2);
    tiger::ConfigVar<TestObject::ptr>::ptr obj_cfg = tiger::Config::Lookup("test.obj", (TestObject::ptr)def_obj, "obj");
    tiger::Config::LoadFromYmal("test", "../tests/test_config.yml");
    TIGER_LOG_D(LOG_NAME) << obj_cfg;
}

int main() {
    test_load_from_path();
    test_object();
    return 0;
}