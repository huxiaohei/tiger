/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/16
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/db/redis/redis_connection_pool.h"

void test_key() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestKey", true, 2);
    auto conns_pool = tiger::redis::RedisConnectionPool::Create("127.0.0.1", 6401, "liuhu", 100, false);
    iom->schedule([conns_pool, iom]() {
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PING();
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SET("hello", "I'm tiger");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TYPE("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TYPE("non exist");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXISTS("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXISTS("non exist");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->RENAME("hello", "world");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->RENAMENX("world", "hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PEXPIREAT("hello", tiger::Millisecond() + 10000);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PTTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXPIREAT("hello", tiger::Second() + 100);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PEXPIRE("hello", 4000);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PTTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXPIRE("hello", 400);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PERSIST("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->RANDOMKEY();
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->DUMP("hello");

        auto all_keys = conns_pool->KEYS("*");
        for (auto &it : all_keys) {
            conns_pool->DEL(it);
        }
        iom->stop();
    });
    iom->start();
}

void test_string() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestString", true, 2);
    auto conns_pool = tiger::redis::RedisConnectionPool::Create("127.0.0.1", 6401, "liuhu", 100, false);
    iom->schedule([conns_pool, iom]() {
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SETNX("hello", "world");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SETNX("hello nx", "world");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<std::string>("hello nx");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<std::string>("tiger");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MSET(3, "name", "tiger", "where", "github", "age", "two years old");
        auto values = conns_pool->MGET<std::string>(3, "name", "where", "age");
        for (auto &it : values) {
            TIGER_LOG_D(tiger::TEST_LOG) << it;
        }
        std::vector<std::pair<std::string, std::string>> k_v_s{{"version", "0.0.1"}};
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MSET(k_v_s);
        values = conns_pool->MGET<std::string>({"name", "version"});
        for (auto &it : values) {
            TIGER_LOG_D(tiger::TEST_LOG) << it;
        }

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MSETNX(3, "name nx", "tiger", "where nx", "github", "age nx", "two years old");
        values = conns_pool->MGET<std::string>(3, "name nx", "where nx", "age nx");
        for (auto &it : values) {
            TIGER_LOG_D(tiger::TEST_LOG) << it;
        }
        k_v_s = {{"version nx", "0.0.3"}};
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MSETNX(k_v_s);
        values = conns_pool->MGET<std::string>({"name nx", "version nx"});
        for (auto &it : values) {
            TIGER_LOG_D(tiger::TEST_LOG) << it;
        }

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SETEX("setex", 1000, "setex");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TTL("setex");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PSETEX("setex", 1000000, "setex");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PTTL("setex");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SET("setbit", "setbit");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SETBIT("setbit", 1, 0);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GETBIT("getbit", 1);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<std::string>("setbit");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SET("setrange", "setrange");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SETRANGE("setrange", 3, " range");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GETRANGE("setrange", 3, 9);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<std::string>("setrange");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SET("append", "append");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->APPEND("append", "append");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<std::string>("append");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->INCR("incr");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<int>("incr");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->INCRBY("incr", 100);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<int>("incr");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->INCRBYFLOAT("incrbyfloat", 0.1);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<double>("incrbyfloat");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->INCRBYFLOAT("incrbyfloat", 0.233);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<double>("incrbyfloat");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<int>("incr");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->DECR("incr");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<int>("incr");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->DECRBY("incr", 10);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GET<int>("incr");

        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->GETSET<std::string>("hello", "tiger");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->STRLEN("hello");

        auto all_keys = conns_pool->KEYS("*");
        for (auto &it : all_keys) {
            conns_pool->DEL(it);
        }
        iom->stop();
    });
    iom->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("RedisTestMianThread");
    test_key();
    test_string();
    return 0;
}