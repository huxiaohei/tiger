/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_UTIL_H__
#define __TIGER_UTIL_H__

#include <chrono>
#include <iostream>
#include <string>

namespace tiger {

bool IsValidName(const std::string &name);

time_t Millisecond();
time_t Second();

namespace FS {
static bool Unlink(const std::string &filename, bool exist = false);

}

}  // namespace tiger

#endif