/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "stream.h"

namespace tiger {

int Stream::read_fixed_size(void *buffer, size_t len) {
    size_t offset = 0;
    int64_t left = len;
    while (left > 0) {
        int64_t read_len = read((char *)buffer + offset, left);
        if (read_len <= 0) {
            return read_len;
        }
        offset += read_len;
        left -= read_len;
    }
    return len;
}

int Stream::read_fixed_size(ByteArray::ptr ba, size_t len) {
    int64_t left = len;
    while (left > 0) {
        int64_t read_len = read(ba, left);
        if (read_len <= 0) {
            return read_len;
        }
        left -= read_len;
    }
    return len;
}

int Stream::write_fixed_size(const void *buffer, size_t len) {
    size_t offset = 0;
    int64_t left = len;
    while (left > 0) {
        int64_t write_len = write((const char *)buffer + offset, left);
        if (write_len <= 0) {
            return write_len;
        }
        offset += write_len;
        left -= write_len;
    }
    return len;
}

int Stream::write_fixed_size(ByteArray::ptr ba, size_t len) {
    int64_t left = len;
    while (left > 0) {
        int64_t write_len = write(ba, left);
        if (len <= 0) {
            return write_len;
        }
        left -= write_len;
    }
    return len;
}

}  // namespace tiger