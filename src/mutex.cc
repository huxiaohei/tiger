/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "mutex.h"

#include "const.h"
#include "macro.h"

namespace tiger
{

Semaphore::Semaphore(uint32_t count)
{
    if (sem_init(&m_semaphore, 0, count))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[sem_init fail]";
        throw std::logic_error("[sem_init fail]");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}

void Semaphore::wait()
{
    if (sem_wait(&m_semaphore))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[sem_wait fail]";
        throw std::logic_error("[sem_wait fail]");
    }
}

void Semaphore::post()
{
    if (sem_post(&m_semaphore))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[sem_post fail]";
        throw std::logic_error("[sem_post fail]");
    }
}

SpinLock::SpinLock()
{
    pthread_spin_init(&m_lock, PTHREAD_PROCESS_SHARED);
}

SpinLock::~SpinLock()
{
    pthread_spin_destroy(&m_lock);
}

void SpinLock::lock()
{
    pthread_spin_lock(&m_lock);
}

void SpinLock::unlock()
{
    pthread_spin_unlock(&m_lock);
}

MutexLock::MutexLock()
{
    pthread_mutex_init(&m_lock, nullptr);
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&m_lock);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&m_lock);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&m_lock);
}

ReadWriteLock::ReadWriteLock()
{
    pthread_rwlock_init(&m_rwlock, nullptr);
}

ReadWriteLock::~ReadWriteLock()
{
    pthread_rwlock_destroy(&m_rwlock);
}

void ReadWriteLock::rdlock()
{
    pthread_rwlock_rdlock(&m_rwlock);
}

void ReadWriteLock::wrlock()
{
    pthread_rwlock_wrlock(&m_rwlock);
}

void ReadWriteLock::unlock()
{
    pthread_rwlock_unlock(&m_rwlock);
}

} // namespace tiger
