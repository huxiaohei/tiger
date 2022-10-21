/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "util.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace tiger {

bool IsValidName(const std::string &name) {
    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789.") == std::string::npos)
        return true;
    return false;
}

time_t Millisecond() {
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

time_t Second() {
    auto tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

namespace FS {

static int __lstat(const char *file, struct stat *st = nullptr) {
    struct stat lst;
    int rst = lstat(file, &lst);
    if (st) {
        *st = lst;
    }
    return rst;
}

bool Unlink(const std::string &filename, bool exist) {
    if (!exist && __lstat(filename.c_str()))
        ;
    return ::unlink(filename.c_str()) == 0;
}

}  // namespace FS

}  // namespace tiger
