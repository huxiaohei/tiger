/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/27
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_MUTEX_H__
#define __TIGER_MUTEX_H__

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

namespace tiger {

class Semaphore {
   private:
    sem_t m_semaphore;

   public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void post();

   private:
    Semaphore(const Semaphore &) = delete;
    Semaphore(const Semaphore &&) = delete;
    Semaphore &operator=(const Semaphore &) = delete;
};

template <typename T>
class ScopedLockTmpl {
   private:
    T &m_mutex;
    bool m_locked;

   public:
    ScopedLockTmpl(T &mutex)
        : m_mutex(mutex), m_locked(false) {
        lock();
    }

    ~ScopedLockTmpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
};

class SpinLock {
   private:
    pthread_spinlock_t m_lock;

   public:
    typedef ScopedLockTmpl<SpinLock> Lock;

    SpinLock();
    ~SpinLock();

    void lock();
    void unlock();
};

class MutexLock {
   private:
    pthread_mutex_t m_lock;

   public:
    typedef ScopedLockTmpl<MutexLock> Lock;

    MutexLock();
    ~MutexLock();

    void lock();
    void unlock();
};

template <typename T>
class ReadScopedLockTmpl {
   private:
    T &m_rlock;
    bool m_locked;

   public:
    ReadScopedLockTmpl(T &rlock)
        : m_rlock(rlock), m_locked(false) {
        lock();
    }

    ~ReadScopedLockTmpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_rlock.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_rlock.unlock();
            m_locked = false;
        }
    }
};

template <typename T>
class WriteScopedLockTmpl {
   private:
    T &m_wrlock;
    bool m_locked;

   public:
    WriteScopedLockTmpl(T &wrlock)
        : m_wrlock(wrlock), m_locked(false) {
        lock();
    }

    ~WriteScopedLockTmpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_wrlock.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_wrlock.unlock();
            m_locked = false;
        }
    }
};

class ReadWriteLock {
   private:
    pthread_rwlock_t m_rwlock;

   public:
    typedef ReadScopedLockTmpl<ReadWriteLock> ReadLock;
    typedef WriteScopedLockTmpl<ReadWriteLock> WriteLock;

    ReadWriteLock();
    ~ReadWriteLock();

    void rdlock();
    void wrlock();
    void unlock();
};

}  // namespace tiger

#endif