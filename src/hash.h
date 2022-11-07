/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/03
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HASH_H__
#define __TIGER_HASH_H__

#include <string>

namespace tiger {

std::string MD5(const std::string &s);
std::string SHA1(const std::string &s);
std::string SHA256(const std::string &s);
std::string SHA512(const std::string &s);

std::string Base64Decode(const std::string &src);
std::string Base64Encode(const std::string &data);
std::string Base64Encode(const void *data, size_t len);

}  // namespace tiger

#endif