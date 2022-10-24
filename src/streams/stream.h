/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_STREAM_H__
#define __TIGER_STREAM_H__

#include <memory>

#include "../bytearray.h"

namespace tiger {

class Stream {
   public:
    typedef std::shared_ptr<Stream> ptr;

    virtual ~Stream(){};

   public:
    virtual int read_fixed_size(void *buffer, size_t len);
    virtual int read_fixed_size(ByteArray::ptr ba, size_t len);
    virtual int write_fixed_size(const void *buffer, size_t len);
    virtual int write_fixed_size(ByteArray::ptr ba, size_t len);

   public:
    virtual int read(void *buffer, size_t len) = 0;
    virtual int read(ByteArray::ptr ba, size_t len) = 0;
    virtual int write(const void *buffer, size_t len) = 0;
    virtual int write(ByteArray::ptr ba, size_t len) = 0;
    virtual void close() = 0;
};

}  // namespace tiger

#endif
