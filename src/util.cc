/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "tiger.h"

namespace tiger {

time_t Millisecond() {
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

time_t Second() {
    auto tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

}  // namespace tiger
