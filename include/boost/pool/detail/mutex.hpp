// Copyright (C) 2000 Stephen Cleary
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.

#ifndef BOOST_POOL_MUTEX_HPP
#define BOOST_POOL_MUTEX_HPP

#include <boost/config.hpp>  // for workarounds

/*!
\file
\brief Extremely Light-Weight wrapper classes for OS thread synchronization.
\details detail/mutex.hpp provides several mutex types that provide a
consistent interface for OS-supplied mutex types.
These are all thread-level mutexes; interprocess mutexes are not supported.

Configuration\n
This header file will try to guess what kind of system it is on.
It will auto-configure itself for Win32 or POSIX+pthread systems.
To stub out all mutex code, bypassing the auto-configuration,
define BOOST_NO_MT before any inclusion of this header.
To prevent ODR violations, this should be defined
in every translation unit in your project, including any library files.

Note:
Each mutex is always either owned or unowned.
If owned, then it is owned by a particular thread.
To "lock" a mutex means to wait until the mutex is unowned,
and then make it owned by the current thread.
To "unlock" a mutex means to release ownership from the current thread
(note that the current thread must own the mutex to release that ownership!).
As a special case, the null_mutex never waits.
*/
// Configuration: for now, we just choose between pthread or Win32 mutexes or none.

#define BOOST_MUTEX_HELPER_NONE         0
#define BOOST_MUTEX_HELPER_WIN32        1
#define BOOST_MUTEX_HELPER_PTHREAD      2

#if !defined(BOOST_HAS_THREADS) && !defined(BOOST_NO_MT)
# define BOOST_NO_MT
#endif

#ifdef BOOST_NO_MT
  // No multithreading -> make locks into no-ops
  #define BOOST_MUTEX_HELPER BOOST_MUTEX_HELPER_NONE
#else
  #ifdef BOOST_WINDOWS
    #define BOOST_MUTEX_HELPER BOOST_MUTEX_HELPER_WIN32
  #else
    #if defined(BOOST_HAS_UNISTD_H)
      #include <unistd.h>
    #endif
    #if defined(_POSIX_THREADS) || defined(BOOST_HAS_PTHREADS)
      #define BOOST_MUTEX_HELPER BOOST_MUTEX_HELPER_PTHREAD
    #endif
  #endif
#endif

#ifndef BOOST_MUTEX_HELPER
  #error Unable to determine platform mutex type; define BOOST_NO_MT to assume single-threaded
#endif

#ifndef BOOST_NO_MT
# ifdef BOOST_WINDOWS
#  include <windows.h>
# endif
# if defined(_POSIX_THREADS) || defined(BOOST_HAS_PTHREADS)
#  include <pthread.h>
# endif
#endif

namespace boost {

namespace details {
namespace pool {

#ifndef BOOST_NO_MT

#ifdef BOOST_WINDOWS

class win32_mutex
{
  private:
    ::CRITICAL_SECTION mtx;

    win32_mutex(const win32_mutex &);
    void operator=(const win32_mutex &);

  public:
    win32_mutex()
    { ::InitializeCriticalSection(&mtx); }

    ~win32_mutex()
    { ::DeleteCriticalSection(&mtx); }

    void lock()
    { ::EnterCriticalSection(&mtx); }

    void unlock()
    { ::LeaveCriticalSection(&mtx); }
};

#endif // defined(BOOST_WINDOWS)

#if defined(_POSIX_THREADS) || defined(BOOST_HAS_PTHREADS)

class pthread_mutex
{
  private:
    ::pthread_mutex_t mtx;

    pthread_mutex(const pthread_mutex &);
    void operator=(const pthread_mutex &);

  public:
    pthread_mutex()
    { ::pthread_mutex_init(&mtx, 0); }

    ~pthread_mutex()
    { ::pthread_mutex_destroy(&mtx); }

    void lock()
    { ::pthread_mutex_lock(&mtx); }

    void unlock()
    { ::pthread_mutex_unlock(&mtx); }
};

#endif // defined(_POSIX_THREADS) || defined(BOOST_HAS_PTHREADS)

#endif // !defined(BOOST_NO_MT)

class null_mutex
{
  private:
    null_mutex(const null_mutex &);
    void operator=(const null_mutex &);

  public:
    null_mutex() { }

    static void lock() { }
    static void unlock() { }
};

#if BOOST_MUTEX_HELPER == BOOST_MUTEX_HELPER_NONE
  typedef null_mutex default_mutex;
#elif BOOST_MUTEX_HELPER == BOOST_MUTEX_HELPER_WIN32
  typedef win32_mutex default_mutex;
#elif BOOST_MUTEX_HELPER == BOOST_MUTEX_HELPER_PTHREAD
  typedef pthread_mutex default_mutex;
#endif

} // namespace pool
} // namespace details

} // namespace boost

#undef BOOST_MUTEX_HELPER_WIN32
#undef BOOST_MUTEX_HELPER_PTHREAD
#undef BOOST_MUTEX_HELPER_NONE
#undef BOOST_MUTEX_HELPER

#endif
