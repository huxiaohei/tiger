/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_BYTEARRAY_H__
#define __TIGER_BYTEARRAY_H__

#include <sys/socket.h>

#include <memory>
#include <vector>

namespace tiger
{

class ByteArray
{
  private:
    struct Node
    {
        Node();
        Node(size_t s);
        ~Node();

        char *ptr;
        Node *next;
        size_t size;
    };

  private:
    size_t m_base_size;
    size_t m_position;
    size_t m_capacity;
    size_t m_size;
    int8_t m_endian;
    Node *m_root;
    Node *m_cur;

  private:
    void add_free_capacity_to(size_t size);

  public:
    typedef std::shared_ptr<ByteArray> ptr;

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

  public:
    void write_fixed_int8(const int8_t &v);
    void write_fixed_uint8(const uint8_t &v);

    void write_fixed_int16(int16_t v);
    void write_fixed_uint16(uint16_t v);

    void write_fixed_int32(int32_t v);
    void write_fixed_uint32(uint32_t v);

    void write_fixed_int64(int64_t v);
    void write_fixed_uint64(uint64_t v);

    void write_int32(int32_t v);
    void write_uint32(uint32_t v);

    void write_int64(int64_t v);
    void write_uint64(uint64_t v);

    void write_float(float v);
    void write_double(double v);

    void write_fixed_str16(const std::string &v);
    void write_fixed_str32(const std::string &v);
    void write_fixed_str64(const std::string &v);

    void write_str(const std::string &v);
    void write_str_without_len(const std::string &v);

    int8_t read_fixed_int8();
    uint8_t read_fixed_uint8();

    int16_t read_fixed_int16();
    uint16_t read_fixed_uint16();

    int32_t read_fixed_int32();
    uint32_t read_fixed_uint32();

    int64_t read_fixed_int64();
    uint64_t read_fixed_uint64();

    int32_t read_int32();
    uint32_t read_uint32();

    int64_t read_int64();
    uint64_t read_uint64();

    float read_float();
    double read_double();

    std::string read_fixed_str16();
    std::string read_fixed_str32();
    std::string read_fixed_str64();

    std::string read_str();

    void clear();
    void write(const void *buf, size_t size);
    bool write_to_file(const std::string &name) const;

    void read(void *buf, size_t size);
    void read(void *buf, size_t size, size_t position) const;
    bool read_from_file(const std::string &name);

    std::string to_string() const;
    std::string to_hex_string() const;

  public:
    size_t get_base_size() const
    {
        return m_base_size;
    }
    size_t get_position() const
    {
        return m_position;
    }
    size_t get_enable_read_size() const
    {
        return m_size - m_position;
    }
    size_t get_size() const
    {
        return m_size;
    }
    size_t get_capacity() const
    {
        return m_capacity;
    }
    size_t get_free_capacity() const
    {
        return m_capacity - m_position;
    }

    size_t get_enable_read_buffers(std::vector<iovec> &buffers, size_t len = ~0ull) const;
    size_t get_enable_read_buffers(std::vector<iovec> &buffers, size_t len, size_t position) const;
    size_t get_enable_write_buffers(std::vector<iovec> &buffers, size_t len);

    bool is_little_endian() const;
    void set_position(size_t v);
    void set_little_endian(bool v);
};

} // namespace tiger

#endif
