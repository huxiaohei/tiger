/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/13
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/macro.h"
#include "../src/streams/zlib_stream.h"
#include "../src/util.h"

void test_zlib()
{
    std::string data = tiger::StringUtils::Random(1024000);
    auto gzip_compress = tiger::ZlibStream::CreateZlib(true);
    TIGER_LOG_D(tiger::TEST_LOG) << "[compress:" << gzip_compress->write(data.c_str(), data.size())
                                 << " length:" << gzip_compress->get_buffers().size()
                                 << " flush:" << gzip_compress->flush() << "]";

    auto compress_str = gzip_compress->get_result();
    auto gzip_uncompress = tiger::ZlibStream::CreateZlib(false);
    TIGER_LOG_D(tiger::TEST_LOG) << "[uncompress:" << gzip_uncompress->write(compress_str.c_str(), compress_str.size())
                                 << " length:" << gzip_uncompress->get_buffers().size()
                                 << " flush:" << gzip_uncompress->flush() << "]";
    TIGER_ASSERT_WITH_INFO(gzip_uncompress->get_result() == data, "zlib test error");
}

void test_deflate()
{
    std::string data = tiger::StringUtils::Random(1024000);
    auto deflate_compress = tiger::ZlibStream::CreateDeflate(true);
    TIGER_LOG_D(tiger::TEST_LOG) << "[compress:" << deflate_compress->write(data.c_str(), data.size())
                                 << " length:" << deflate_compress->get_buffers().size()
                                 << " flush:" << deflate_compress->flush() << "]";

    auto compress_str = deflate_compress->get_result();
    auto deflate_uncompress = tiger::ZlibStream::CreateDeflate(false);
    TIGER_LOG_D(tiger::TEST_LOG) << "[uncompress:"
                                 << deflate_uncompress->write(compress_str.c_str(), compress_str.size())
                                 << " length:" << deflate_uncompress->get_buffers().size()
                                 << " flush:" << deflate_uncompress->flush() << "]";
    TIGER_ASSERT_WITH_INFO(deflate_uncompress->get_result() == data, "deflate test error");
}

void test_gzip()
{
    std::string data = tiger::StringUtils::Random(10);
    auto gzip_compress = tiger::ZlibStream::CreateGzip(true);
    TIGER_LOG_D(tiger::TEST_LOG) << "[compress:" << gzip_compress->write(data.c_str(), data.size())
                                 << " length:" << gzip_compress->get_buffers().size() << " data:" << data
                                 << " flush:" << gzip_compress->flush() << "]";

    auto compress_str = gzip_compress->get_result();
    auto gzip_uncompress = tiger::ZlibStream::CreateGzip(false);
    TIGER_LOG_D(tiger::TEST_LOG) << "[uncompress:" << gzip_uncompress->write(compress_str.c_str(), compress_str.size())
                                 << " length:" << gzip_uncompress->get_buffers().size()
                                 << " flush:" << gzip_uncompress->flush() << "]";
    TIGER_ASSERT_WITH_INFO(gzip_uncompress->get_result() == data, "deflate test error");
}

int main()
{
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    TIGER_LOG_D(tiger::TEST_LOG) << "[zlib_stream test start]";
    // test_zlib();
    // test_deflate();
    test_gzip();
    TIGER_LOG_D(tiger::TEST_LOG) << "[zlib_stream test end]";
    return 0;
}