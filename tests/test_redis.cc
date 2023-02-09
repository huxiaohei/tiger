/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/db/redis/redis_connection.h"

void test_redis_connection() {
    auto redis_connection = tiger::redis::RedisConnection::CreateRedisConnection("127.0.0.1", 6401, "liuhu");
    auto rst = redis_connection->exec_cmd<tiger::redis::RedisResultStr>("GET hello");
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    test_redis_connection();
    return 0;
}