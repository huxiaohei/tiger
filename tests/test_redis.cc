/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/db/redis/redis_connection_pool.h"

void test_redis_connection_str() {
    auto redis_connection = tiger::redis::RedisConnection::Create("127.0.0.1", 6401, "liuhu");
    auto rst = redis_connection->exec_cmd<tiger::redis::RedisResultVector<std::string>>("MGET hello world");
    TIGER_LOG_D(tiger::TEST_LOG) << rst;
}

void test_redis_connection_vector() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestSubThread", true, 2);
    auto redis_connection = tiger::redis::RedisConnection::Create("127.0.0.1", 6401, "liuhu");
    iom->schedule([redis_connection, iom]() {
        auto rst = redis_connection->exec_cmd<tiger::redis::RedisResultVector<std::string>>("KEYS *");
        for (const auto &it : rst->get_data()) {
            auto del_rst = redis_connection->exec_cmd<tiger::redis::RedisResultInt>("DEL " + it);
            TIGER_LOG_D(tiger::TEST_LOG) << del_rst;
        }
        iom->stop();
    });
    iom->start();
}

void test_redis_connection_pool() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestSubThread", true, 4);
    auto redis_connection_pool = tiger::redis::RedisConnectionPool::Create("127.0.0.1", 6401, "liuhu", 10);
    for (int i = 0; i < 5000; ++i) {
        iom->add_timer(i, [redis_connection_pool, i]() {
            redis_connection_pool->get_connection()->exec_cmd<tiger::redis::RedisResultStr>("SET idx" + std::to_string(i) + " " + std::to_string(i));
        });
    }
    iom->add_timer(15000, [iom, redis_connection_pool]() {
        iom->stop();
    });
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "redis conn client cnt:" << redis_connection_pool->get_conn_total();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("RedisTestMianThread");
    test_redis_connection_str();
    // test_redis_connection_vector();
    // test_redis_connection_pool();
    return 0;
}