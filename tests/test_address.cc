/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/20
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

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
        TIGER_LOG_D(tiger::TEST_LOG) << "\nname:" << it.first
                                     << "\nip:" << it.second.first
                                     << "\nprefixlen:" << it.second.second
                                     << "\nsubnetmask:" << it.second.first->subnet_mask(it.second.second)
                                     << "\nnetworkAddress:" << it.second.first->network_address(it.second.second)
                                     << "\nbroadcastAddress:" << it.second.first->broadcast_address(it.second.second);
    }
    std::vector<std::pair<tiger::IPAddress::ptr, uint32_t>> lo_rts;
    if (!tiger::IPAddress::InterfaceAddresses(lo_rts, "lo", AF_INET)) {
        TIGER_LOG_D(tiger::TEST_LOG) << "get lo interface address fail";
    }
    for (auto it : lo_rts) {
        TIGER_LOG_D(tiger::TEST_LOG) << "\nname:lo"
                                     << "\nip:" << it.first
                                     << "\nprefixlen:" << it.second
                                     << "\nsubnetmask:" << it.first->subnet_mask(it.second)
                                     << "\nnetworkAddress:" << it.first->network_address(it.second)
                                     << "\nbroadcastAddress:" << it.first->broadcast_address(it.second);
    }
    TIGER_LOG_D(tiger::TEST_LOG) << "test iface end";
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    TIGER_LOG_D(tiger::TEST_LOG) << "address test start";
    test_lookup();
    test_iface();
    TIGER_LOG_D(tiger::TEST_LOG) << "address test end";
    return 0;
}