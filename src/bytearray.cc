/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "bytearray.h"

#include <math.h>
#include <string.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "endian.h"
#include "macro.h"

namespace tiger
{

static uint32_t Encode_Zigzag32(const int32_t &v)
{
    if (v < 0)
    {
        return ((uint32_t)(-v) << 1) - 1;
    }
    else
    {
        return (v << 1);
    }
}

static uint64_t Encode_Zigzag64(const int64_t &v)
{
    if (v < 0)
    {
        return ((uint64_t)(-v) << 1) - 1;
    }
    else
    {
        return (v << 1);
    }
}

static int32_t Decode_Zigzag32(const uint32_t &v)
{
    return (v >> 1) ^ -(v & 1);
}

static int64_t Decode_Zigzag64(const uint64_t &v)
{
    return (v >> 1) ^ -(v & 1);
}

ByteArray::Node::Node() : ptr(nullptr), next(nullptr), size(0)
{
}

ByteArray::Node::Node(size_t s) : ptr(new char[s]), next(nullptr), size(s)
{
}

ByteArray::Node::~Node()
{
    if (ptr)
        delete[] ptr;
    ptr = nullptr;
    next = nullptr;
}

ByteArray::ByteArray(size_t base_size)
    : m_base_size(base_size), m_position(0), m_capacity(0), m_size(0), m_endian(__TIGER_BIG_ENDIAN),
      m_root(new Node(base_size)), m_cur(m_root)
{
}

ByteArray::~ByteArray()
{
    Node *tmp = m_root;
    while (tmp)
    {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_root = nullptr;
    m_cur = nullptr;
}

void ByteArray::add_free_capacity_to(size_t size)
{
    size_t free_cap = get_free_capacity();
    if (free_cap > size)
        return;
    size -= free_cap;
    size_t cnt = ceil(1.0 * size / m_base_size) + 1;
    Node *tmp = m_root;
    while (tmp->next)
    {
        tmp = tmp->next;
    }
    while (cnt > 0)
    {
        tmp->next = new Node(m_base_size);
        tmp = tmp->next;
        m_capacity += m_base_size;
        --cnt;
    }
    if (free_cap == 0 && cnt > 0)
    {
        m_cur = m_cur->next;
    }
}

void ByteArray::write_fixed_int8(const int8_t &v)
{
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_uint8(const uint8_t &v)
{
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_int16(int16_t v)
{
    if (m_endian != __TIGER_BYTE_ORDER)
    {
        v = byteswap(v);
    }
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_uint16(uint16_t v)
{
    if (m_endian != __TIGER_BYTE_ORDER)
    {
        v = byteswap(v);
    }
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_int32(int32_t v)
{
    if (m_endian != __TIGER_BYTE_ORDER)
    {
        v = byteswap(v);
    }
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_uint32(uint32_t v)
{
    if (m_endian != __TIGER_BYTE_ORDER)
    {
        v = byteswap(v);
    }
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_int64(int64_t v)
{
    if (m_endian != __TIGER_BYTE_ORDER)
    {
        v = byteswap(v);
    }
    write(&v, sizeof(v));
}

void ByteArray::write_fixed_uint64(uint64_t v)
{
    if (m_endian != __TIGER_BYTE_ORDER)
    {
        v = byteswap(v);
    }
    write(&v, sizeof(v));
}

void ByteArray::write_int32(int32_t v)
{
    write_uint32(Encode_Zigzag32(v));
}

void ByteArray::write_uint32(uint32_t v)
{
    uint8_t tmp[5];
    uint8_t i = 0;
    while (v >= 0x80)
    {
        tmp[i++] = (v & 0x7F) | 0x80;
        v >>= 7;
    }
    tmp[i++] = v;
    write(tmp, i);
}

void ByteArray::write_int64(int64_t v)
{
    write_uint64(Encode_Zigzag64(v));
}

void ByteArray::write_uint64(uint64_t v)
{
    uint8_t tmp[10];
    uint8_t i = 0;
    while (v >= 0x80)
    {
        tmp[i++] = (v & 0x7F) | 0x80;
        v >>= 7;
    }
    tmp[i++] = v;
    write(tmp, i);
}

void ByteArray::write_float(float v)
{
    uint32_t value = 0;
    memcpy(&value, &v, sizeof(v));
    write_fixed_uint32(value);
}

void ByteArray::write_double(double v)
{
    uint64_t value = 0;
    memcpy(&value, &v, sizeof(v));
    write_fixed_uint64(value);
}

void ByteArray::write_fixed_str16(const std::string &v)
{
    write_fixed_uint16(v.size());
    write(v.c_str(), v.size());
}

void ByteArray::write_fixed_str32(const std::string &v)
{
    write_fixed_uint32(v.size());
    write(v.c_str(), v.size());
}

void ByteArray::write_fixed_str64(const std::string &v)
{
    write_fixed_uint64(v.size());
    write(v.c_str(), v.size());
}

void ByteArray::write_str(const std::string &v)
{
    write_uint64(v.size());
    write(v.c_str(), v.size());
}

void ByteArray::write_str_without_len(const std::string &v)
{
    write(v.c_str(), v.size());
}

int8_t ByteArray::read_fixed_int8()
{
    int8_t v = 0;
    read(&v, sizeof(v));
    return v;
}

uint8_t ByteArray::read_fixed_uint8()
{
    uint8_t v = 0;
    read(&v, sizeof(v));
    return v;
}

int16_t ByteArray::read_fixed_int16()
{
    int16_t v = 0;
    read(&v, sizeof(v));
    if (m_endian == __TIGER_BYTE_ORDER)
    {
        return v;
    }
    else
    {
        return byteswap(v);
    }
}

uint16_t ByteArray::read_fixed_uint16()
{
    uint16_t v = 0;
    read(&v, sizeof(v));
    if (m_endian == __TIGER_BYTE_ORDER)
    {
        return v;
    }
    else
    {
        return byteswap(v);
    }
}

int32_t ByteArray::read_fixed_int32()
{
    int32_t v = 0;
    read(&v, sizeof(v));
    if (m_endian == __TIGER_BYTE_ORDER)
    {
        return v;
    }
    else
    {
        return byteswap(v);
    }
}

uint32_t ByteArray::read_fixed_uint32()
{
    uint32_t v = 0;
    read(&v, sizeof(v));
    if (m_endian == __TIGER_BYTE_ORDER)
    {
        return v;
    }
    else
    {
        return byteswap(v);
    }
}

int64_t ByteArray::read_fixed_int64()
{
    int64_t v = 0;
    read(&v, sizeof(v));
    if (m_endian == __TIGER_BYTE_ORDER)
    {
        return v;
    }
    else
    {
        return byteswap(v);
    }
}

uint64_t ByteArray::read_fixed_uint64()
{
    uint64_t v = 0;
    read(&v, sizeof(v));
    if (m_endian == __TIGER_BYTE_ORDER)
    {
        return v;
    }
    else
    {
        return byteswap(v);
    }
}

int32_t ByteArray::read_int32()
{
    return Decode_Zigzag32(read_uint32());
}

uint32_t ByteArray::read_uint32()
{
    uint32_t v = 0;
    for (uint8_t i = 0; i < 32; i += 7)
    {
        uint8_t b = read_fixed_uint8();
        if (b < 0x80)
        {
            v |= (((uint32_t)b) << i);
            break;
        }
        else
        {
            v |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return v;
}

int64_t ByteArray::read_int64()
{
    return Decode_Zigzag64(read_uint64());
}

uint64_t ByteArray::read_uint64()
{
    uint64_t v = 0;
    for (uint8_t i = 0; i < 64; i += 7)
    {
        uint8_t b = read_fixed_int8();
        if (b < 0x80)
        {
            v |= (((uint64_t)b) << i);
            break;
        }
        else
        {
            v |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return v;
}

float ByteArray::read_float()
{
    uint32_t v = read_fixed_uint32();
    float value = 0.0;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::read_double()
{
    uint64_t v = read_fixed_uint64();
    double value = 0.0;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::read_fixed_str16()
{
    uint16_t len = read_fixed_uint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::read_fixed_str32()
{
    uint32_t len = read_fixed_uint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::read_fixed_str64()
{
    uint64_t len = read_fixed_uint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::read_str()
{
    uint64_t len = read_uint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

void ByteArray::clear()
{
    m_position = 0;
    m_size = 0;
    m_capacity = m_base_size;
    Node *tmp = m_root->next;
    while (tmp)
    {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void *buf, size_t size)
{
    if (size == 0)
        return;
    add_free_capacity_to(size);
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    size_t has_write = 0;
    while (size > 0)
    {
        if (ncap >= size)
        {
            memcpy(m_cur->ptr + npos, (const char *)buf + has_write, size);
            if (m_cur->size == (npos + size) && m_cur->next)
            {
                m_cur = m_cur->next;
            }
            m_position += size;
            has_write += size;
            size = 0;
        }
        else
        {
            memcpy(m_cur->ptr + npos, (const char *)buf + has_write, ncap);
            m_position += ncap;
            has_write += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
    if (m_position > m_size)
    {
        m_size = m_position;
    }
}

bool ByteArray::write_to_file(const std::string &name) const
{
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[write to file fail"
                                << " name:" << name << " errno:" << strerror(errno);
        return false;
    }
    size_t size = get_enable_read_size();
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    Node *cur = m_cur;
    while (size > 0)
    {
        if (ncap >= size)
        {
            ofs.write(cur->ptr + npos, size);
            size = 0;
        }
        else
        {
            ofs.write(cur->ptr + npos, ncap);
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
    ofs.close();
    return true;
}

void ByteArray::read(void *buf, size_t size)
{
    if (size > get_enable_read_size())
    {
        throw std::out_of_range("[read out of range]");
    }
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    size_t has_read = 0;
    while (size > 0)
    {
        if (ncap >= size)
        {
            memcpy((char *)buf + has_read, m_cur->ptr + npos, size);
            if (m_cur->size == (npos + size) && m_cur->next)
            {
                m_cur = m_cur->next;
            }
            m_position += size;
            has_read += size;
            size = 0;
        }
        else
        {
            memcpy((char *)buf + has_read, m_cur->ptr + npos, ncap);
            m_position += ncap;
            has_read += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
}

void ByteArray::read(void *buf, size_t size, size_t position) const
{
    if (size > (m_size - position))
    {
        throw std::out_of_range("[read not of range]");
    }
    size_t npos = position % m_base_size;
    Node *cur = m_root;
    while (position >= cur->size)
    {
        position -= cur->size;
        cur = cur->next;
    }
    size_t ncap = cur->size - npos;
    size_t has_read = 0;
    while (size > 0)
    {
        if (ncap >= size)
        {
            memcpy((char *)buf + has_read, cur->ptr + npos, size);
            if (cur->size == (npos + size) && cur->next)
            {
                cur = cur->next;
            }
            position += size;
            has_read += size;
            size = 0;
        }
        else
        {
            memcpy((char *)buf + has_read, cur->ptr + npos, ncap);
            position += ncap;
            has_read += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
}

bool ByteArray::read_from_file(const std::string &name)
{
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[read from file fail"
                                << " name:" << name << " errno:" << strerror(errno) << "]";
        return false;
    }
    std::shared_ptr<char> buff(new char[m_base_size], [](char *ptr) { delete[] ptr; });
    while (!ifs.eof())
    {
        ifs.read(buff.get(), m_base_size);
        write(buff.get(), ifs.gcount());
    }
    ifs.close();
    return true;
}

std::string ByteArray::to_string() const
{
    std::string str;
    str.resize(get_enable_read_size());
    if (str.empty())
    {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::to_hex_string() const
{
    const std::string &str = to_string();
    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (i > 0 && i % 32 == 0)
        {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

size_t ByteArray::get_enable_read_buffers(std::vector<iovec> &buffers, size_t len) const
{
    len = len > get_enable_read_size() ? get_enable_read_size() : len;
    if (len == 0)
        return 0;
    size_t read_size = len;
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node *cur = m_cur;
    while (len > 0)
    {
        if (ncap >= len)
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return read_size;
}

size_t ByteArray::get_enable_read_buffers(std::vector<iovec> &buffers, size_t len, size_t position) const
{
    if (position >= m_size)
        return 0;
    len = len > (m_size - position) ? (m_size - position) : len;
    if (len == 0)
        return 0;
    size_t read_size = len;
    Node *_cur = m_root;
    while (position >= _cur->size)
    {
        position -= _cur->size;
        _cur = _cur->next;
    }
    struct iovec iov;
    size_t ncap = _cur->size - position;
    while (len > 0)
    {
        if (ncap >= len)
        {
            iov.iov_base = _cur->ptr + position;
            iov.iov_len = len;
            len = 0;
        }
        else
        {
            iov.iov_base = _cur->ptr + position;
            iov.iov_len = ncap;
            len -= ncap;
            _cur = _cur->next;
            ncap = _cur->size;
            position = 0;
        }
        buffers.push_back(iov);
    }
    return read_size;
}

size_t ByteArray::get_enable_write_buffers(std::vector<iovec> &buffers, size_t len)
{
    if (len == 0)
        return 0;
    add_free_capacity_to(len);
    size_t write_size = len;
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node *cur = m_cur;
    while (len > 0)
    {
        if (ncap >= len)
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else
        {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return write_size;
}

bool ByteArray::is_little_endian() const
{
    return m_endian == __TIGER_LITTLE_ENDIAN;
}

void ByteArray::set_position(size_t v)
{
    if (v > m_capacity)
    {
        throw std::out_of_range("[set position out of range]");
    }
    m_position = v;
    m_size = m_position > m_size ? m_position : m_size;
    m_cur = m_root;
    while (v > m_cur->size)
    {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    if (v == m_cur->size && m_cur->next)
    {
        m_cur = m_cur->next;
    }
}

void ByteArray::set_little_endian(bool v)
{
    if (v)
    {
        m_endian = __TIGER_LITTLE_ENDIAN;
    }
    else
    {
        m_endian = __TIGER_BIG_ENDIAN;
    }
}

} // namespace tiger
