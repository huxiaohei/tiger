/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/02
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_COROUTINE_H__
#define __TIGER_COROUTINE_H__

#include <ucontext.h>

#include "log.h"

namespace tiger {

class Coroutine : public std::enable_shared_from_this<Coroutine> {
   public:
    enum State {
        INIT = 0x1,
        RUNNING = 0x10,
        YIELD = 0x100,
        TERMINAL = 0x1000,
        EXCEPT = 0x10000
    };

   private:
    size_t m_id;
    std::function<void()> m_fn;
    State m_state;
    pid_t m_thread_id;
    ucontext_t m_ctx;
    void *m_stack = nullptr;
    size_t m_stack_size = 0;

   private:
    static void Run();

   public:
    typedef std::shared_ptr<Coroutine> ptr;

    explicit Coroutine();
    explicit Coroutine(std::function<void()> fn, size_t stack_size = 0);
    ~Coroutine();

    const size_t id() const { return m_id; }
    const State state() const { return m_state; }

   public:
    void yield();
    void resume();
    bool reset(std::function<void()> fn);

   public:
    static size_t CurCoroutineId();
    static void Yield();
    static void Resume();
};

}  // namespace tiger

#endif
