/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/03
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "hash.h"

#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <string.h>

#include <iomanip>
#include <sstream>
#include <vector>

namespace tiger
{

inline std::string message_digest(const std::string &s, const EVP_MD *evp_md, size_t digest_length)
{
    std::vector<unsigned char> md(digest_length, 0);
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_MD_CTX_init(ctx);
    EVP_DigestInit_ex(ctx, evp_md, NULL);
    EVP_DigestUpdate(ctx, s.data(), s.size());
    EVP_DigestFinal_ex(ctx, md.data(), NULL);
    EVP_MD_CTX_destroy(ctx);

    std::stringstream ss;
    for (auto c : md)
    {
        ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)c;
    }
    return ss.str();
}

std::string MD5(const std::string &s)
{
    return message_digest(s, EVP_md5(), MD5_DIGEST_LENGTH);
}

std::string SHA256(const std::string &s)
{
    return message_digest(s, EVP_sha256(), SHA256_DIGEST_LENGTH);
}

std::string SHA512(const std::string &s)
{
    return message_digest(s, EVP_sha512(), SHA512_DIGEST_LENGTH);
}

std::string SHA1(const std::string &s)
{
    const unsigned char *buf = (const unsigned char *)(s.c_str());
    unsigned char out[1024];
    ::SHA1(buf, s.size(), out);
    return std::string((char *)out);
}

std::string Base64Decode(const std::string &src)
{
    std::string result;
    result.resize(src.size() * 3 / 4);
    char *writeBuf = &result[0];
    const char *ptr = src.c_str();
    const char *end = ptr + src.size();
    while (ptr < end)
    {
        int i = 0;
        int padding = 0;
        int packed = 0;
        for (; i < 4 && ptr < end; ++i, ++ptr)
        {
            if (*ptr == '=')
            {
                ++padding;
                packed <<= 6;
                continue;
            }
            if (padding > 0)
            {
                return "";
            }
            int val = 0;
            if (*ptr >= 'A' && *ptr <= 'Z')
            {
                val = *ptr - 'A';
            }
            else if (*ptr >= 'a' && *ptr <= 'z')
            {
                val = *ptr - 'a' + 26;
            }
            else if (*ptr >= '0' && *ptr <= '9')
            {
                val = *ptr - '0' + 52;
            }
            else if (*ptr == '+')
            {
                val = 62;
            }
            else if (*ptr == '/')
            {
                val = 63;
            }
            else
            {
                return ""; // invalid character
            }
            packed = (packed << 6) | val;
        }
        if (i != 4)
        {
            return "";
        }
        if (padding > 0 && ptr != end)
        {
            return "";
        }
        if (padding > 2)
        {
            return "";
        }
        *writeBuf++ = (char)((packed >> 16) & 0xff);
        if (padding != 2)
        {
            *writeBuf++ = (char)((packed >> 8) & 0xff);
        }
        if (padding == 0)
        {
            *writeBuf++ = (char)(packed & 0xff);
        }
    }
    result.resize(writeBuf - result.c_str());
    return result;
}

std::string Base64Encode(const std::string &data)
{
    return Base64Encode(data.c_str(), data.size());
}

std::string Base64Encode(const void *data, size_t len)
{
    const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    ret.reserve(len * 4 / 3 + 2);
    const unsigned char *ptr = (const unsigned char *)data;
    const unsigned char *end = ptr + len;
    while (ptr < end)
    {
        unsigned int packed = 0;
        int i = 0;
        int padding = 0;
        for (; i < 3 && ptr < end; ++i, ++ptr)
        {
            packed = (packed << 8) | *ptr;
        }
        if (i == 2)
        {
            padding = 1;
        }
        else if (i == 1)
        {
            padding = 2;
        }
        for (; i < 3; ++i)
        {
            packed <<= 8;
        }
        ret.append(1, base64[packed >> 18]);
        ret.append(1, base64[(packed >> 12) & 0x3f]);
        if (padding != 2)
        {
            ret.append(1, base64[(packed >> 6) & 0x3f]);
        }
        if (padding == 0)
        {
            ret.append(1, base64[packed & 0x3f]);
        }
        ret.append(padding, '=');
    }
    return ret;
}

} // namespace tiger