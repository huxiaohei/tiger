/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/db/redis/redis_connection_pool.h"

void test_redis_connection() {
    auto redis_connection = tiger::redis::RedisConnection::Create("127.0.0.1", 6401, "liuhu");
    auto rst = redis_connection->exec_cmd<tiger::redis::RedisResultStr>("GET hello");
    TIGER_LOG_D(tiger::TEST_LOG) << rst->get_data();
}

void test_redis_connection_pool() {
    auto iom = std::make_shared<tiger::IOManager>("IOManager", true, 5);
    auto redis_connection_pool = tiger::redis::RedisConnectionPool::Create("127.0.0.1", 6401, "liuhu", 10);
    for (int i = 0; i < 1000; ++i) {
        iom->add_timer(i, [redis_connection_pool]() {
            redis_connection_pool->get_connection()->exec_cmd<tiger::redis::RedisResultStr>("GET hello");
        });
    }
    iom->add_timer(1500, [iom]() {
        iom->stop();
    });
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "redis conn client cnt:" << redis_connection_pool->get_conn_total();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    test_redis_connection();
    test_redis_connection_pool();
    return 0;
}