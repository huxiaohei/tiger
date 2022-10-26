/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/bytearray.h"
#include "../src/macro.h"
#include "../src/thread.h"

void test_fix_uint() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_fixed_uint8(1);
    ba->write_fixed_uint8(255);
    ba->write_fixed_uint16(0);
    ba->write_fixed_uint16(255);
    ba->write_fixed_uint16(512);
    ba->write_fixed_uint16(-1);
    ba->write_fixed_uint32(0);
    ba->write_fixed_uint32(255);
    ba->write_fixed_uint32(512);
    ba->write_fixed_uint32(-1);
    ba->write_fixed_uint64(0);
    ba->write_fixed_uint64(255);
    ba->write_fixed_uint64(512);
    ba->write_fixed_uint64(-1);

    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint8:" << unsigned(ba->read_fixed_uint8()) << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint8:" << unsigned(ba->read_fixed_uint8()) << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint16:" << ba->read_fixed_uint16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint16:" << ba->read_fixed_uint16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint16:" << ba->read_fixed_uint16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint16:" << ba->read_fixed_uint16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint32:" << ba->read_fixed_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint32:" << ba->read_fixed_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint32:" << ba->read_fixed_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint32:" << ba->read_fixed_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint64:" << ba->read_fixed_uint64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint64:" << ba->read_fixed_uint64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint64:" << ba->read_fixed_uint64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_uint64:" << ba->read_fixed_uint64() << "]";
}

void test_uint() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_uint32(0);    // 1
    ba->write_uint32(255);  // 2
    ba->write_uint32(512);  // 4
    ba->write_uint32(-1);   // 1
    ba->write_uint64(0);    // 1
    ba->write_uint64(255);  // 2
    ba->write_uint64(512);  // 4
    ba->write_uint64(-1);   // 1

    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint32:" << ba->read_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint32:" << ba->read_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint32:" << ba->read_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint32:" << ba->read_uint32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint64:" << ba->read_uint64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint64:" << ba->read_uint64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint64:" << ba->read_uint64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_uint64:" << ba->read_uint64() << "]";
}

void test_fix_int() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_fixed_int8(-128);
    ba->write_fixed_int8(127);
    ba->write_fixed_int16(0);
    ba->write_fixed_int16(32767);
    ba->write_fixed_int16(-32768);
    ba->write_fixed_int16(-1);
    ba->write_fixed_int32(0);
    ba->write_fixed_int32(2147483647);
    ba->write_fixed_int32(-2147483648);
    ba->write_fixed_int32(-1);
    ba->write_fixed_int64(0);
    ba->write_fixed_int64(10);
    ba->write_fixed_int64(-10);
    ba->write_fixed_int64(-1);

    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int8:" << signed(ba->read_fixed_int8()) << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int8:" << signed(ba->read_fixed_int8()) << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int16:" << ba->read_fixed_int16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int16:" << ba->read_fixed_int16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int16:" << ba->read_fixed_int16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int16:" << ba->read_fixed_int16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int32:" << ba->read_fixed_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int32:" << ba->read_fixed_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int32:" << ba->read_fixed_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int32:" << ba->read_fixed_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int64:" << ba->read_fixed_int64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int64:" << ba->read_fixed_int64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int64:" << ba->read_fixed_int64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_int64:" << ba->read_fixed_int64() << "]";
}

void test_int() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_int32(0);            // 1
    ba->write_int32(2147483647);   // 5
    ba->write_int32(-2147483648);  // 5
    ba->write_int32(-1);           // 1
    ba->write_int64(0);            // 1
    ba->write_int64(10);           // 1
    ba->write_int64(-10);          // 1
    ba->write_int64(-1);           // 1

    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int32:" << ba->read_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int32:" << ba->read_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int32:" << ba->read_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int32:" << ba->read_int32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int64:" << ba->read_int64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int64:" << ba->read_int64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int64:" << ba->read_int64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_int64:" << ba->read_int64() << "]";
}

void test_float_and_double() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_float(3.1415926);
    ba->write_double(3.1415926);
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    TIGER_LOG_D(tiger::TEST_LOG) << "[read_float:" << ba->read_float() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_double:" << ba->read_double() << "]";
}

void test_str() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_fixed_str16("Hello World 16");  // 2 + 14
    ba->write_fixed_str32("Hello World 32");  // 4 + 14
    ba->write_fixed_str64("Hello World 64");  // 8 + 14
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";
    ba->write_str("hello World");  // 1 + 11
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";
    ba->write_str_without_len("hello World");  // 11
    TIGER_LOG_D(tiger::TEST_LOG) << "[size:" << ba->get_size()
                                 << " capacity:" << ba->get_capacity()
                                 << " enable_read:" << ba->get_enable_read_size()
                                 << " free_capacity:" << ba->get_free_capacity() << "]";

    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_str16:" << ba->read_fixed_str16() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_str32:" << ba->read_fixed_str32() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_fixed_str64:" << ba->read_fixed_str64() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_str:" << ba->read_str() << "]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[to_string:" << ba->to_string() << "]";
}

void test_file() {
    auto ba = std::make_shared<tiger::ByteArray>(4);
    ba->write_str("Hello World");
    ba->set_position(0);
    ba->write_to_file("./bytearry.log");
    ba->clear();
    ba->read_from_file("./bytearry.log");
    ba->set_position(0);
    TIGER_LOG_D(tiger::TEST_LOG) << "[read_from_file:" << ba->read_str() << "]";
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("BYTE_ARRAY");
    TIGER_LOG_D(tiger::TEST_LOG) << "[bytearray test start]";
    test_fix_uint();
    test_uint();
    test_fix_int();
    test_int();
    test_float_and_double();
    test_str();
    test_file();
    TIGER_LOG_D(tiger::TEST_LOG) << "[bytearray test end]";
    return 0;
}