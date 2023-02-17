/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/16
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/db/redis/redis_connection_pool.h"

void test_key() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestSubThread", true, 2);
    auto conns_pool = tiger::redis::RedisConnectionPool::Create("127.0.0.1", 6401, "liuhu", 100, false);
    iom->schedule([conns_pool, iom]() {
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PING();
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SET("hello", "tiger");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TYPE("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->RENAME("hello", "world");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->RENAMENX("world", "hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXISTS("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PEXPIREAT("hello", 1676546288000);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXPIREAT("hello", 1676546288);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PEXPIRE("hello", 10000000);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->EXPIRE("hello", 10000);
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->TTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PERSIST("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->PTTL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->RANDOMKEY();
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->DUMP("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->KEYS("*").size();
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MOVE("hello", "1");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->SET("hello", "tiger");
        // TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->DEL("hello");
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MGET<std::string>(2, "hello", "tiger").size();
        std::vector<std::string> keys{"hello", "world"};
        TIGER_LOG_D(tiger::TEST_LOG) << conns_pool->MGET<std::string>(keys).size();
    });
    iom->add_timer(1000, [iom]() {
        iom->stop();
    });
    iom->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("RedisTestMianThread");
    test_key();
    return 0;
}