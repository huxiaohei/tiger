/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/02
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "coroutine.h"

#include <atomic>

#include "config.h"

namespace tiger {

static std::atomic<size_t> s_co_id{0};
static std::atomic<int64_t> s_co_cnt{0};

static thread_local Coroutine::ptr s_t_running_co = nullptr;
static thread_local Coroutine::ptr s_t_main_co = nullptr;

static ConfigVar<size_t>::ptr g_co_stack_size = Config::Lookup<size_t>("tiger.coroutine.stackSize", 1024 * 128, "Coroutine Stack Size");

class MallocStackAllocator {
   public:
    static void *Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void *vp) {
        return free(vp);
    }
};

size_t Coroutine::CurCoroutineId() {
    if (s_t_running_co) {
        return s_t_running_co->id();
    }
    return 0;
}

std::shared_ptr<Coroutine> Coroutine::GetRunningCo() {
    if (s_t_running_co) {
        return s_t_running_co;
    }
    return s_t_main_co;
}

void Coroutine::Yield() {
    if (s_t_running_co)
        s_t_running_co->yield();
}

void Coroutine::Resume() {
    if (s_t_main_co)
        s_t_main_co->resume();
}

void Coroutine::Run() {
    try {
        s_t_running_co->m_fn();
        s_t_running_co->m_fn = nullptr;
        s_t_running_co->m_state = State::TERMINAL;
    } catch (const std::exception &e) {
        s_t_running_co->m_state = State::EXCEPT;
        TIGER_LOG_E(tiger::SYSTEM_LOG) << "Run Cteoroutine error:\n"
                                       << "id:" << s_t_running_co->m_id << "\n"
                                       << "error:" << e.what();
    } catch (...) {
        s_t_running_co->m_state = State::EXCEPT;
        TIGER_LOG_E(tiger::SYSTEM_LOG) << "Run Coroutine error:\n"
                                       << "id:" << s_t_running_co->m_id;
    }
    swapcontext(&(s_t_running_co->m_ctx), &(s_t_main_co->m_ctx));
}

Coroutine::Coroutine() {
    m_state = State::RUNNING;
    m_id = ++s_co_id;
    if (getcontext(&m_ctx)) {
        TIGER_ASSERT_WITH_INFO(false, "getcontext error");
    }
    ++s_co_cnt;
}

Coroutine::Coroutine(std::function<void()> fn, size_t stack_size)
    : m_fn(fn), m_state(State::INIT) {
    if (!s_t_main_co) {
        s_t_main_co = std::make_shared<Coroutine>();
        s_t_running_co = s_t_main_co;
    }
    m_id = ++s_co_id;
    m_stack_size = stack_size == 0 ? g_co_stack_size->val() : stack_size;
    m_stack = MallocStackAllocator::Alloc(m_stack_size);
    if (getcontext(&m_ctx)) {
        TIGER_ASSERT_WITH_INFO(false, "getcontext error");
    }
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_flags = 0;
    m_ctx.uc_stack.ss_size = m_stack_size;
    makecontext(&m_ctx, &Coroutine::Run, 0);
    ++s_co_cnt;
}

Coroutine::~Coroutine() {
    if (m_stack) {
        MallocStackAllocator::Dealloc(m_stack);
        m_stack = nullptr;
    } else {
        s_t_main_co.reset();
        s_t_running_co.reset();
    }
    --s_co_cnt;
}

void Coroutine::yield() {
    TIGER_ASSERT_WITH_INFO(s_t_running_co != s_t_main_co, "The main coroutinue cannot be suspended by itself");
    m_state = State::YIELD;
    s_t_main_co->m_state = State::RUNNING;
    s_t_running_co = s_t_main_co;
    if (swapcontext(&m_ctx, &(s_t_main_co->m_ctx))) {
        TIGER_ASSERT_WITH_INFO(false, "swapcontext error");
    }
}

void Coroutine::resume() {
    TIGER_ASSERT_WITH_INFO(m_state & (State::INIT | State::YIELD), "Coroutine can only be resumed when it is INIT or Yield");
    s_t_main_co->m_state = State::YIELD;
    m_state = State::RUNNING;
    s_t_running_co = shared_from_this();
    if (swapcontext(&(s_t_main_co->m_ctx), &m_ctx)) {
        TIGER_ASSERT_WITH_INFO(false, "swapcontext error");
    }
    s_t_main_co->m_state = State::RUNNING;
    s_t_running_co = s_t_main_co;
}

bool Coroutine::reset(std::function<void()> fn) {
    TIGER_ASSERT_WITH_INFO(m_stack, "Coroutine stack is nullptr");
    if (m_state & (State::INIT | State::TERMINAL | State::EXCEPT)) {
        m_fn = fn;
        if (getcontext(&m_ctx)) {
            TIGER_ASSERT_WITH_INFO(false, "getcontext error");
        }
        m_id = ++s_co_id;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stack_size;
        m_state = State::INIT;
        makecontext(&m_ctx, &Coroutine::Run, 0);
        return true;
    }
    return false;
}

}  // namespace tiger