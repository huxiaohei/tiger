/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/17
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "address.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stddef.h>

#include <iomanip>

#include "endian.h"
#include "macro.h"

namespace tiger {

template <typename T>
static uint32_t CountBytes(T value) {
    uint32_t r = 0;
    while (value) {
        value &= value - 1;
        ++r;
    }
    return r;
}

template <typename T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

Address::ptr Address::Create(const sockaddr *addr) {
    if (addr == nullptr) return nullptr;
    Address::ptr r;
    switch (addr->sa_family) {
        case AF_INET:
            r = std::make_shared<IPv4Address>(*(const sockaddr_in *)addr);
            break;
        case AF_INET6:
            r = std::make_shared<IPv6Address>(*(const sockaddr_in6 *)addr);
            break;
        default:
            r.reset(new UnknownAddress(*addr));
            break;
    }
    return r;
}

bool Address::Lookup(std::vector<Address::ptr> &r, const std::string &host,
                     int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;

    std::string node;
    const char *service = nullptr;
    if (!host.empty() && host[0] == '[') {
        const char *endipv6 = (const char *)memchr(host.c_str() + 1, ']', host.size() - 1);
        if (endipv6) {
            if (endipv6 < host.c_str() + host.size() && *(endipv6 + 1) == ':') {
                service = endipv6 + 2;
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }
    if (node.empty()) {
        service = (const char *)memchr(host.c_str(), ':', host.size());
        if (service) {
            if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }
    if (node.empty()) {
        node = host;
    }
    int err = getaddrinfo(node.c_str(), service, &hints, &results);
    if (err) {
        TIGER_LOG_E(SYSTEM_LOG) << "[getaddeinfo faills "
                                << " host:" << host
                                << " family:" << family
                                << " type:" << type
                                << " protocol:" << protocol
                                << " error:" << gai_strerror(err) << "]";
        return false;
    }
    next = results;
    while (next) {
        r.push_back(Create(next->ai_addr));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    return !r.empty();
}

Address::ptr Address::LookupAny(const std::string &host, int family,
                                int type, int protocol) {
    std::vector<Address::ptr> r;
    if (Lookup(r, host, family, type, protocol)) {
        return r[0];
    }
    return nullptr;
}

IPAddress::ptr IPAddress::LookupAny(const std::string &host, int family,
                                    int type, int protocol) {
    std::vector<Address::ptr> rst;
    if (Lookup(rst, host, family, type, protocol)) {
        for (auto &i : rst) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if (v) return v;
        }
    }
    return nullptr;
}

bool IPAddress::InterfaceAddresses(std::multimap<std::string, std::pair<IPAddress::ptr, uint32_t>> &r,
                                   int family) {
    struct ifaddrs *next, *rs;
    if (getifaddrs(&rs) != 0) {
        TIGER_LOG_E(SYSTEM_LOG) << "[getifaddrs error"
                                << " errno:" << strerror(errno) << "]";
        return false;
    }
    try {
        next = rs;
        while (next) {
            IPAddress::ptr addr;
            uint32_t prefix_len = ~0u;
            if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                next = next->ifa_next;
                continue;
            }
            switch (next->ifa_addr->sa_family) {
                case AF_INET: {
                    addr = std::make_shared<IPv4Address>(*(const sockaddr_in *)next->ifa_addr);
                    uint32_t net = ((sockaddr_in *)next->ifa_netmask)->sin_addr.s_addr;
                    prefix_len = CountBytes(net);
                    break;
                }
                case AF_INET6: {
                    addr = std::make_shared<IPv6Address>(*(const sockaddr_in6 *)next->ifa_addr);
                    struct in6_addr &net = ((sockaddr_in6 *)next->ifa_netmask)->sin6_addr;
                    prefix_len = 0;
                    for (int i = 0; i < 16; ++i) {
                        prefix_len += CountBytes(net.s6_addr[i]);
                    }
                    break;
                }
                default:
                    break;
            }
            if (addr) {
                r.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
            }
            next = next->ifa_next;
        }
    } catch (...) {
        TIGER_LOG_E(SYSTEM_LOG) << "[InterfaceAddress error]";
        freeifaddrs(rs);
        return false;
    }
    freeifaddrs(rs);
    return !r.empty();
}

bool IPAddress::InterfaceAddresses(std::vector<std::pair<IPAddress::ptr, uint32_t>> &r,
                                   const std::string &iface, int family) {
    if (iface.empty() || iface == "*") {
        if (family == AF_INET || family == AF_UNSPEC) {
            r.push_back(std::make_pair(std::make_shared<IPv4Address>(), 0u));
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            r.push_back(std::make_pair(std::make_shared<IPv6Address>(), 0u));
        }
        return true;
    }
    std::multimap<std::string, std::pair<IPAddress::ptr, uint32_t>> rs;
    if (!InterfaceAddresses(rs, family)) {
        return false;
    }
    auto it = rs.equal_range(iface);
    while (it.first != it.second) {
        r.push_back(it.first->second);
        ++it.first;
    }
    return !rs.empty();
}

int Address::get_family() const {
    return get_addr()->sa_family;
}

std::string Address::to_string() const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &other) const {
    socklen_t min_len = std::min(get_addr_len(), other.get_addr_len());
    int r = memcmp(get_addr(), other.get_addr(), min_len);
    return r == 0 ? get_addr_len() < other.get_addr_len() : r < 0;
}

bool Address::operator==(const Address &other) const {
    return get_addr_len() == other.get_addr_len() &&
           memcmp(get_addr(), other.get_addr(), get_addr_len()) == 0;
}

bool Address::operator!=(const Address &other) const {
    return !(*this == other);
}

IPv4Address::IPv4Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
}

IPv4Address::IPv4Address(const sockaddr_in &addr)
    : m_addr(addr) {
}

IPv4Address::IPv4Address(uint32_t addr, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = bswap_on_little_endian(port);
    m_addr.sin_addr.s_addr = addr;
}

IPv4Address::ptr IPv4Address::Create(const char *addr, uint16_t port) {
    IPv4Address::ptr rt = std::make_shared<IPv4Address>();
    rt->m_addr.sin_port = bswap_on_little_endian(port);
    int err = inet_pton(AF_INET, addr, &rt->m_addr.sin_addr);
    if (err <= 0) {
        TIGER_LOG_E(SYSTEM_LOG) << "[inet_pton error"
                                << " addr:" << addr
                                << " err:" << err
                                << " errno:" << strerror(errno) << "]";
        return nullptr;
    }
    return rt;
}

const sockaddr *IPv4Address::get_addr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *IPv4Address::get_addr() {
    return (sockaddr *)&m_addr;
}

const socklen_t IPv4Address::get_addr_len() const {
    return sizeof(m_addr);
}

std::ostream &IPv4Address::insert(std::ostream &os) const {
    uint32_t addr = bswap_on_little_endian(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff) << ":" << get_port();
    return os;
}

IPAddress::ptr IPv4Address::broadcast_address(uint32_t prefix_len) {
    if (prefix_len > 32) return nullptr;
    sockaddr_in broadcast_addr(m_addr);
    broadcast_addr.sin_addr.s_addr |= bswap_on_little_endian(CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(broadcast_addr);
}

IPAddress::ptr IPv4Address::network_address(uint32_t prefix_len) {
    if (prefix_len > 32) return nullptr;
    sockaddr_in network_addr(m_addr);
    network_addr.sin_addr.s_addr &= bswap_on_little_endian(~CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(network_addr);
}

IPAddress::ptr IPv4Address::subnet_mask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~bswap_on_little_endian(CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(subnet);
}

uint32_t IPv4Address::get_port() const {
    return bswap_on_little_endian(m_addr.sin_port);
}

void IPv4Address::set_port(uint16_t v) {
    m_addr.sin_port = bswap_on_little_endian(v);
}

IPv6Address::IPv6Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6 &addr) {
    m_addr = addr;
}

IPv6Address::IPv6Address(const uint8_t addr[16], uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = bswap_on_little_endian(port);
}

IPv6Address::ptr IPv6Address::Create(const char *addr, uint16_t port) {
    IPv6Address::ptr rt = std::make_shared<IPv6Address>();
    rt->m_addr.sin6_port = bswap_on_little_endian(port);
    int r = inet_pton(AF_INET6, addr, &rt->m_addr.sin6_addr);
    if (r <= 0) {
        TIGER_LOG_E(SYSTEM_LOG) << "[inet_pton error"
                                << " rt:" << rt
                                << " erron:" << strerror(errno) << "]";
        return nullptr;
    }
    return rt;
}

const sockaddr *IPv6Address::get_addr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *IPv6Address::get_addr() {
    return (sockaddr *)&m_addr;
}

const socklen_t IPv6Address::get_addr_len() const {
    return sizeof(m_addr);
}

std::ostream &IPv6Address::insert(std::ostream &os) const {
    os << "[";
    uint16_t *addr = (uint16_t *)m_addr.sin6_addr.s6_addr;
    bool used_zero = false;
    for (size_t i = 0; i < 8; ++i) {
        if (addr[i] == 0 && !used_zero) {
            continue;
        }
        if (i && addr[i - 1] == 0 && !used_zero) {
            os << ":";
            used_zero = true;
        }
        if (i) {
            os << ":";
        }
        os << std::hex << (int)bswap_on_little_endian(addr[i]) << std::dec;
    }
    if (!used_zero && addr[7] == 0) {
        os << "::";
    }
    os << "]" << bswap_on_little_endian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcast_address(uint32_t prefix_len) {
    sockaddr_in6 broadcasr_addr(m_addr);
    broadcasr_addr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i) {
        broadcasr_addr.sin6_addr.s6_addr[i] = 0xff;
    }
    return std::make_shared<IPv6Address>(broadcasr_addr);
}

IPAddress::ptr IPv6Address::network_address(uint32_t prefix_len) {
    sockaddr_in6 network_addr(m_addr);
    network_addr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i) {
        network_addr.sin6_addr.s6_addr[i] = 0x00;
    }
    return std::make_shared<IPv6Address>(network_addr);
}

IPAddress::ptr IPv6Address::subnet_mask(uint32_t prefix_len) {
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
    for (uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return std::make_shared<IPv6Address>(subnet);
}

uint32_t IPv6Address::get_port() const {
    return bswap_on_little_endian(m_addr.sin6_port);
}

void IPv6Address::set_port(uint16_t v) {
    m_addr.sin6_port = bswap_on_little_endian(v);
}

UnixAddress::UnixAddress() {
    static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_len = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string &path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_len = path.size() + 1;
    if (!path.empty() && path[0] != '\0') {
        --m_len;
    }
    if (m_len > sizeof(m_addr.sun_path)) {
        throw std::logic_error("[path too long " + path + "]");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_len);
    m_len += offsetof(sockaddr_un, sun_path);
}

const sockaddr *UnixAddress::get_addr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *UnixAddress::get_addr() {
    return (sockaddr *)&m_addr;
}

const socklen_t UnixAddress::get_addr_len() const {
    return m_len;
}

std::ostream &UnixAddress::insert(std::ostream &os) const {
    if (m_len > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1, m_len - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

void UnixAddress::set_addr_len(uint32_t len) {
    m_len = len;
}

std::string UnixAddress::get_path() const {
    std::stringstream ss;
    if (m_len > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') {
        ss << "\\0" << std::string(m_addr.sun_path + 1, m_len - offsetof(sockaddr_un, sun_path) - 1);
    } else {
        ss << m_addr.sun_path;
    }
    return ss.str();
}

UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr &addr) {
    m_addr = addr;
}

const sockaddr *UnknownAddress::get_addr() const {
    return &m_addr;
}
sockaddr *UnknownAddress::get_addr() {
    return &m_addr;
}

const socklen_t UnknownAddress::get_addr_len() const {
    return sizeof(m_addr);
}

std::ostream &UnknownAddress::insert(std::ostream &os) const {
    os << "[UnknownAddress family:" << m_addr.sa_family << "]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Address &addr) {
    addr.insert(os);
    return os;
}

std::ostream &operator<<(std::ostream &os, const Address::ptr addr) {
    addr->insert(os);
    return os;
}

}  // namespace tiger