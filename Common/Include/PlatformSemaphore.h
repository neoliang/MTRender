#ifndef __PLATFORMSEMAPHORE_H
#define __PLATFORMSEMAPHORE_H

#ifdef _WIN32

#else
	#include <semaphore.h>
#endif
#	include <errno.h>



class PlatformSemaphore
{
    friend class Semaphore;
protected:
	void Create();
	void Destroy();

	void WaitForSignal();
	void Signal();

private:
#ifdef _WIN32
	HANDLE	m_Semaphore;
#else
	sem_t m_Semaphore;
#endif
};

inline void PlatformSemaphore::Create() { 
#ifdef _WIN32
	m_Semaphore = CreateSemaphoreA(NULL, 0, 256, NULL);
#else
	if (sem_init(&m_Semaphore, 0, 0) == -1) ;
#endif
}
inline void PlatformSemaphore::Destroy() { 
#ifdef _WIN32
	CloseHandle(m_Semaphore);
#else
	if(sem_destroy(&m_Semaphore) == -1);
#endif
}
inline void PlatformSemaphore::WaitForSignal() { 
#ifdef _WIN32
	while (1)
	{
		DWORD result = WaitForSingleObjectEx(m_Semaphore, INFINITE, TRUE);
		switch (result)
		{
		case WAIT_OBJECT_0:
			// We got the signal
			return;
		case WAIT_IO_COMPLETION:
			// Allow thread to run IO completion task
			Sleep(1);
			break;
		default:
			break;
		}
}
#else
	if (sem_wait(&m_Semaphore) == -1); 
#endif
}
inline void PlatformSemaphore::Signal() {
#ifdef _WIN32
	ReleaseSemaphore(m_Semaphore, 1, NULL);
#else
	if (sem_post(&m_Semaphore) == -1);
#endif
}

class Semaphore
{
public:
    Semaphore() { m_Semaphore.Create(); }
    ~Semaphore() { m_Semaphore.Destroy(); }
    void Reset() { m_Semaphore.Destroy(); m_Semaphore.Create(); }
    void WaitForSignal() { m_Semaphore.WaitForSignal(); }
    void Signal() { m_Semaphore.Signal(); }
    
private:
    PlatformSemaphore m_Semaphore;
};
#endif // __PLATFORMSEMAPHORE_H
