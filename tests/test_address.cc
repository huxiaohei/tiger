/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/20
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/address.h"
#include "../src/macro.h"
#include "../src/thread.h"

void test_lookup() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test lookup start";
    std::vector<tiger::Address::ptr> rsts;
    if (!tiger::IPAddress::Lookup(rsts, "www.qq.com", AF_INET6, SOCK_STREAM)) {
        TIGER_LOG_E(tiger::TEST_LOG) << "test lookup fail";
        return;
    }
    if (!tiger::IPAddress::Lookup(rsts, "www.baidu.com", AF_INET, SOCK_STREAM)) {
        TIGER_LOG_E(tiger::TEST_LOG) << "test lookup fail";
        return;
    }
    auto it = tiger::IPAddress::LookupAny("www.github.com", AF_INET, SOCK_STREAM);
    if (it) {
        rsts.push_back(it);
    }
    for (auto it : rsts) {
        TIGER_LOG_D(tiger::TEST_LOG) << it;
    }
    TIGER_LOG_D(tiger::TEST_LOG) << "test lookup end";
}

void test_iface() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test iface start";
    std::multimap<std::string, std::pair<tiger::IPAddress::ptr, uint32_t>> rts;
    if (!tiger::IPAddress::InterfaceAddresses(rts, AF_INET)) {
        TIGER_LOG_D(tiger::TEST_LOG) << "get IPV4 interface address fail";
        return;
    }
    if (!tiger::IPAddress::InterfaceAddresses(rts, AF_INET6)) {
        TIGER_LOG_D(tiger::TEST_LOG) << "get IPV6 interface address fail";
        return;
    }
    for (auto it : rts) {
        TIGER_LOG_D(tiger::TEST_LOG) << "[name:" << it.first
                                     << " ip:" << it.second.first->to_string()
                                     << " prefixlen:" << it.second.second
                                     << " subnetmask:" << it.second.first->subnet_mask(it.second.second)->to_string()
                                     << " networkAddress:" << it.second.first->network_address(it.second.second)->to_string()
                                     << " broadcastAddress:" << it.second.first->broadcast_address(it.second.second)->to_string() << "]";
    }
    std::vector<std::pair<tiger::IPAddress::ptr, uint32_t>> lo_rts;
    if (!tiger::IPAddress::InterfaceAddresses(lo_rts, "lo", AF_INET)) {
        TIGER_LOG_D(tiger::TEST_LOG) << "get lo interface address fail";
    }
    for (auto it : lo_rts) {
        TIGER_LOG_D(tiger::TEST_LOG) << "[name:lo"
                                     << " ip:" << it.first->to_string()
                                     << " prefixlen:" << it.second
                                     << " subnetmask:" << it.first->subnet_mask(it.second)->to_string()
                                     << " networkAddress:" << it.first->network_address(it.second)->to_string()
                                     << " broadcastAddress:" << it.first->broadcast_address(it.second)->to_string() << "]";
    }
    TIGER_LOG_D(tiger::TEST_LOG) << "test iface end";
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("ADDRESS");
    TIGER_LOG_D(tiger::TEST_LOG) << "[address test start]";
    test_lookup();
    test_iface();
    TIGER_LOG_D(tiger::TEST_LOG) << "[address test end]";
    return 0;
}