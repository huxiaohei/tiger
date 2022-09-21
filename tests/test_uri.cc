/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/uri/uri.h"

int main() {
    tiger::Uri::ptr uri = tiger::Uri::Create("http://baidu.com");
    std::cout << uri << std::endl;
    return 0;
}