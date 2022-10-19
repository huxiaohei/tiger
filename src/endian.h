/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_ENDIAN_H__
#define __TIGER_ENDIAN_H__

#include <byteswap.h>
#include <stdint.h>

#define __TIGER_LITTLE_ENDIAN 1
#define __TIGER_BIG_ENDIAN 2

namespace tiger {

template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define __TIGER_BYTE_ORDER __TIGER_BIG_ENDIAN
#else
#define __TIGER_BYTE_ORDER __TIGER_LITTLE_ENDIAN
#endif

#if __TIGER_BYTE_ORDER == __TIGER_BIG_ENDIAN

template <typename T>
T bswap_on_little_endian(T t) {
    return t;
}

template <typename T>
T bswap_on_big_endian(T t) {
    return byteswap(t);
}

#else

template <typename T>
T bswap_on_little_endian(T t) {
    return byteswap(t);
}

template <typename T>
T bswap_on_big_endian(T t) {
    return t;
}

#endif

}  // namespace tiger

#endif