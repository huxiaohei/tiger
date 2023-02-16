/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/db/redis/redis_connection_pool.h"

void test_redis_connection_str() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestSubThread", true, 2);
    iom->schedule([iom]() {
        auto redis_connection = tiger::redis::RedisConnection::Create("127.0.0.1", 6401, "liuhu");
        auto rst = redis_connection->exec_cmd<tiger::redis::RedisResultVal<std::string>>("GET hello");
        TIGER_LOG_D(tiger::TEST_LOG) << rst;
    });
    iom->add_timer(1000, [iom]() {
        iom->stop();
    });
    iom->start();
}

void test_redis_connection_vector() {
    auto iom = std::make_shared<tiger::IOManager>("RedisTestSubThread", true, 2);
    auto redis_connection = tiger::redis::RedisConnection::Create("127.0.0.1", 6401, "liuhu");
    iom->schedule([redis_connection, iom]() {
        auto rst = redis_connection->exec_cmd<tiger::redis::RedisResultVector<std::string>>("KEYS *");
        for (const auto &it : rst->get_data()) {
            auto del_rst = redis_connection->exec_cmd<tiger::redis::RedisResultVal<int>>("DEL " + it);
            TIGER_LOG_D(tiger::TEST_LOG) << del_rst;
        }
    });
    iom->add_timer(10000, [iom]() {
        iom->stop();
    });
    iom->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("RedisTestMianThread");
    // test_redis_connection_str();
    test_redis_connection_vector();
    return 0;
}