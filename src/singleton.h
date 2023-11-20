/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_SINGLETON_H__
#define __TIGER_SINGLETON_H__

#include <memory>

namespace tiger
{

template <typename T> class Singleton
{
  public:
    static T *Instance()
    {
        static T v;
        return &v;
    }
};

template <typename T> class SingletonPtr
{
  public:
    static std::shared_ptr<T> Instance()
    {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

} // namespace tiger
#endif
