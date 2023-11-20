/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/16
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_ADDRESS_H__
#define __TIGER_ADDRESS_H__

#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tiger
{

class Address
{
  public:
    typedef std::shared_ptr<Address> ptr;

    virtual ~Address(){};

  public:
    static Address::ptr Create(const sockaddr *addr);
    static bool Lookup(std::vector<Address::ptr> &r, const std::string &host, int family = AF_INET, int type = 0,
                       int protocol = 0);
    static Address::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

  public:
    int get_family() const;
    std::string to_string() const;

  public:
    virtual const sockaddr *get_addr() const = 0;
    virtual sockaddr *get_addr() = 0;
    virtual const socklen_t get_addr_len() const = 0;
    virtual std::ostream &insert(std::ostream &os) const = 0;

  public:
    bool operator<(const Address &other) const;
    bool operator==(const Address &other) const;
    bool operator!=(const Address &other) const;
    friend std::ostream &operator<<(std::ostream &os, const Address &addr);
    friend std::ostream &operator<<(std::ostream &os, const Address::ptr addr);
};

class IPAddress : public Address
{
  public:
    typedef std::shared_ptr<IPAddress> ptr;

    virtual ~IPAddress(){};

  public:
    static IPAddress::ptr Create(const char *address, uint16_t port = 0);

  public:
    static IPAddress::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
    static bool InterfaceAddresses(std::multimap<std::string, std::pair<IPAddress::ptr, uint32_t>> &r,
                                   int family = AF_INET);
    static bool InterfaceAddresses(std::vector<std::pair<IPAddress::ptr, uint32_t>> &r, const std::string &iface,
                                   int family = AF_INET);
    virtual IPAddress::ptr broadcast_address(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr network_address(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnet_mask(uint32_t prefix_len) = 0;
    virtual uint32_t get_port() const = 0;
    virtual void set_port(uint16_t v) = 0;
};

class IPv4Address : public IPAddress
{
  private:
    sockaddr_in m_addr;

  public:
    typedef std::shared_ptr<IPv4Address> ptr;

    IPv4Address();
    IPv4Address(const sockaddr_in &addr);
    IPv4Address(uint32_t addr, uint16_t port = 0);

  public:
    static IPv4Address::ptr Create(const char *addr, uint16_t port = 0);

  public:
    const sockaddr *get_addr() const override;
    sockaddr *get_addr() override;
    const socklen_t get_addr_len() const override;
    std::ostream &insert(std::ostream &os) const override;
    IPAddress::ptr broadcast_address(uint32_t prefix_len) override;
    IPAddress::ptr network_address(uint32_t prefix_len) override;
    IPAddress::ptr subnet_mask(uint32_t prefix_len) override;
    uint32_t get_port() const override;
    void set_port(uint16_t v) override;
};

class IPv6Address : public IPAddress
{
  private:
    sockaddr_in6 m_addr;

  public:
    typedef std::shared_ptr<IPv6Address> ptr;

    IPv6Address();
    IPv6Address(const sockaddr_in6 &addr);
    IPv6Address(const uint8_t addr[16], uint16_t port = 0);

  public:
    static IPv6Address::ptr Create(const char *addr, uint16_t port = 0);

  public:
    const sockaddr *get_addr() const override;
    sockaddr *get_addr() override;
    const socklen_t get_addr_len() const override;
    std::ostream &insert(std::ostream &os) const override;
    IPAddress::ptr broadcast_address(uint32_t prefix_len) override;
    IPAddress::ptr network_address(uint32_t prefix_len) override;
    IPAddress::ptr subnet_mask(uint32_t prefix_len) override;
    uint32_t get_port() const override;
    void set_port(uint16_t v) override;
};

class UnixAddress : public Address
{
  private:
    sockaddr_un m_addr;
    socklen_t m_len;

  public:
    typedef std::shared_ptr<UnixAddress> ptr;

    UnixAddress();
    UnixAddress(const std::string &path);

  public:
    const sockaddr *get_addr() const override;
    sockaddr *get_addr() override;
    const socklen_t get_addr_len() const override;
    std::ostream &insert(std::ostream &os) const override;

  public:
    void set_addr_len(uint32_t len);
    std::string get_path() const;
};

class UnknownAddress : public Address
{
  private:
    sockaddr m_addr;

  public:
    typedef std::shared_ptr<UnknownAddress> ptr;

    UnknownAddress(int family);
    UnknownAddress(const sockaddr &addr);

  public:
    const sockaddr *get_addr() const override;
    sockaddr *get_addr() override;
    const socklen_t get_addr_len() const override;
    std::ostream &insert(std::ostream &os) const override;
};

} // namespace tiger

#endif