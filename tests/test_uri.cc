/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

int main() {
    tiger::URI::ptr uri = tiger::URI::Create("http://www.baidu.com");
    std::cout << uri << std::endl;
    return 0;
}