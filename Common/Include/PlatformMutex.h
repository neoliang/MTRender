#ifndef __PLATFORMMUTEX_H
#define __PLATFORMMUTEX_H


#ifdef _WIN32

#else
#include <pthread.h>
#endif

#if defined(__native_client__)
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif


class PlatformMutex 
{
	friend class Mutex;
protected:
	PlatformMutex()
	{

#ifdef _WIN32
		InitializeCriticalSection(&crit_sec);
#else
		pthread_mutexattr_t    attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&mutex, &attr);
		pthread_mutexattr_destroy(&attr);
#endif

	}
	~PlatformMutex()
	{
#ifdef _WIN32
		DeleteCriticalSection(&crit_sec);
#else
		pthread_mutex_destroy(&mutex);
#endif
	}

	void Lock()
	{
#ifdef _WIN32
		EnterCriticalSection(&crit_sec);
#else
		pthread_mutex_lock(&mutex);
#endif
	}
	void Unlock()
	{
#ifdef _WIN32
		LeaveCriticalSection(&crit_sec);
#else
		pthread_mutex_unlock(&mutex);
#endif
	}
	bool TryLock()
	{
#ifdef _WIN32
		return TryEnterCriticalSection(&crit_sec) ? true : false;
#else
		return pthread_mutex_trylock(&mutex) == 0;
#endif
	}

private:
#ifdef _WIN32
	CRITICAL_SECTION crit_sec;
#else
	pthread_mutex_t mutex;
#endif
};

class Mutex 
{
public:

	class AutoLock
	{
	public:
		AutoLock(Mutex& mutex)
			: m_Mutex(&mutex)
		{
			mutex.Lock();
		}

		~AutoLock()
		{
			m_Mutex->Unlock();
		}

	private:
		AutoLock(const AutoLock&);
		AutoLock& operator=(const AutoLock&);

	private:
		Mutex * m_Mutex;
	};

	Mutex() {};
	~Mutex() {};

	void Lock() { m_Mutex.Lock(); }
	void Unlock() { m_Mutex.Unlock(); }

	// Returns true if locking succeeded
	bool TryLock()
	{
		return m_Mutex.TryLock();
	}

	// Returns true if the mutex is currently locked
	bool IsLocked()
	{
		if (m_Mutex.TryLock())
		{
			Unlock();
			return false;
		}
		else
		{
			return true;
		}
	}

	void BlockUntilUnlocked()
	{
		Lock();
		Unlock();
	}

private:

	PlatformMutex	m_Mutex;
};
#endif // __PLATFORMMUTEX_H
