
#include "esUtil.h"
#include "RingBuffer.h"
#include "PlatformMutex.h"
#include "PlatformSemaphore.h"

#define min(a,b)            (((a) < (b)) ? (a) : (b))


RingBuffer::RingBuffer(size_t size)
{
	SetDefaults();
	Create(size);
}

RingBuffer::~RingBuffer()
{
	Destroy();
}


void RingBuffer::Create(size_t size)
{
	m_Reader = &m_Header.reader;
	m_Writer = &m_Header.writer;
	if (size != 0)
		m_Buffer = new char[size];
	m_BufferSize = size;
	m_Reader->Reset();
	m_Writer->Reset();
	m_Writer->bufferEnd = size;

	m_Mutex = new Mutex;
	m_ReadSemaphore = new Semaphore;
	m_WriteSemaphore = new Semaphore;
	
}


void RingBuffer::Destroy()
{
	if (m_Buffer == NULL) return;
	delete m_Buffer;
	m_Reader->Reset();
	m_Writer->Reset();

	delete m_Mutex;
	delete m_ReadSemaphore;
	delete m_WriteSemaphore;
	
	SetDefaults();
}

RingBuffer::size_t RingBuffer::GetCurrentSize() const
{
	return m_BufferSize;
}

const void*	RingBuffer::GetBuffer() const
{
	//Assert(m_Mode != kModeThreaded);
	return m_Buffer;
}

void RingBuffer::ReadStreamingData(void* data, size_t size, size_t alignment, size_t step)
{
	// This should not be size_t, as the GfxDevice may run across processes of different
	// bitness, and the data serialized in the command buffer must match.
	size_t sz = ReadValueType<UInt32>();

	char* dest = (char*)data;
	for (size_t offset = 0; offset < size; offset += step)
	{
		size_t bytes = min(size - offset, step);
		const void* src = GetReadDataPointer(bytes, alignment);
		if (data)
			memcpy(dest, src, bytes);

		int magic = ReadValueType<int>();

		ReadReleaseData();
		dest += step;
	}
}

void RingBuffer::ReadReleaseData()
{
	if (m_Reader->checkedWraps == m_Reader->bufferWraps)
	{
		// We only update the position
		m_Reader->checkedPos = m_Reader->bufferPos;
	}
	else
	{

		Mutex::AutoLock lock(*m_Mutex);
		m_Reader->checkedPos = m_Reader->bufferPos;
		m_Reader->checkedWraps = m_Reader->bufferWraps;
	}
	SendReadSignal();
}

void RingBuffer::WriteStreamingData(const void* data, size_t size, size_t alignment, size_t step)
{
	// This should not be size_t, as the GfxDevice may run across processes of different
	// bitness, and the data serialized in the command buffer must match.
	WriteValueType<UInt32>(size);

	const char* src = (const char*)data;
	for (size_t offset = 0; offset < size; offset += step)
	{
		size_t bytes = min(size - offset, step);
		void* dest = GetWriteDataPointer(bytes, alignment);
		memcpy(dest, src, bytes);
		WriteValueType<int>(1234);

		// In the NaCl Web Player, make sure that only complete commands are submitted, as we are not truely
		// asynchronous.

		WriteSubmitData();

		src += step;
	}
	WriteSubmitData();
}

void RingBuffer::WriteSubmitData()
{
	if (m_Writer->checkedWraps == m_Writer->bufferWraps)
	{
		// We only update the position
		m_Writer->checkedPos = m_Writer->bufferPos;
	}
	else
	{

		Mutex::AutoLock lock(*m_Mutex);
		m_Writer->checkedPos = m_Writer->bufferPos;
		m_Writer->checkedWraps = m_Writer->bufferWraps;
	}
	SendWriteSignal();
}

void RingBuffer::SetDefaults()
{
	m_Buffer = NULL;
	m_BufferSize = 0;
	m_Mutex = NULL;
	m_ReadSemaphore = NULL;
	m_WriteSemaphore = NULL;
	m_NeedsReadSignal = 0;
	m_NeedsWriteSignal = 0;
};

bool RingBuffer::HasDataToRead() const
{
	if (m_Reader->bufferWraps == m_Writer->checkedWraps)
	{
		return (m_Reader->bufferPos < m_Writer->checkedPos) || (m_Reader->bufferPos < m_Reader->bufferEnd);
	}
	else
		return true;
}

inline int AtomicIncrement(int volatile* i)
{
#if _WIN32
	return  _InterlockedIncrement((long volatile*)i);
#else
	return __sync_add_and_fetch(i, 1);
#endif
}

inline bool AtomicCompareExchange(int volatile* i, int newValue, int expectedValue)
{
#if _WIN32
	return _InterlockedCompareExchange((long volatile*)i, (long)newValue, (long)expectedValue) == expectedValue;
#else
	return __sync_bool_compare_and_swap(i, expectedValue, newValue);
#endif
}

void RingBuffer::HandleReadOverflow(size_t& dataPos, size_t& dataEnd)
{

	Mutex::AutoLock lock(*m_Mutex);

	if (dataEnd > m_BufferSize)
	{
		dataEnd -= dataPos;
		dataPos = 0;
		m_Reader->bufferPos = 0;
		m_Reader->bufferWraps++;
	}

	for (;;)
	{
		// Get how many buffer lengths writer is ahead of reader
		// This may be -1 if we are waiting for the writer to wrap
		size_t comparedPos = m_Writer->checkedPos;
		size_t comparedWraps = m_Writer->checkedWraps;
		size_t wrapDist = comparedWraps - m_Reader->bufferWraps;
		m_Reader->bufferEnd = (wrapDist == 0) ? comparedPos : (wrapDist == 1) ? m_BufferSize : 0;

		if (dataEnd <= m_Reader->bufferEnd)
		{
			break;
		}
		AtomicIncrement(&m_NeedsWriteSignal);

		m_Mutex->Unlock();
		if (comparedPos != m_Writer->checkedPos || comparedWraps != m_Writer->checkedWraps)
		{
			// Writer position changed while we requested a signal
			// Request might be missed, so we signal ourselves to avoid deadlock
			SendWriteSignal();
		}
		SendReadSignal();
		// Wait for writer thread
		m_WriteSemaphore->WaitForSignal();
		m_Mutex->Lock();
	}
}

void RingBuffer::HandleWriteOverflow(size_t& dataPos, size_t& dataEnd)
{



	Mutex::AutoLock lock(*m_Mutex);

	if (dataEnd > m_BufferSize)
	{
		dataEnd -= dataPos;
		dataPos = 0;
		m_Writer->bufferPos = 0;
		m_Writer->bufferWraps++;
	}

	for (;;)
	{
		// Get how many buffer lengths writer is ahead of reader
		// This may be 2 if we are waiting for the reader to wrap
		size_t comparedPos = m_Reader->checkedPos;
		size_t comparedWraps = m_Reader->checkedWraps;
		size_t wrapDist = m_Writer->bufferWraps - comparedWraps;
		m_Writer->bufferEnd = (wrapDist == 0) ? m_BufferSize : (wrapDist == 1) ? comparedPos : 0;

		if (dataEnd <= m_Writer->bufferEnd)
		{
			break;
		}
		AtomicIncrement(&m_NeedsReadSignal);
		m_Mutex->Unlock();
		if (comparedPos != m_Reader->checkedPos || comparedWraps != m_Reader->checkedWraps)
		{
			// Reader position changed while we requested a signal
			// Request might be missed, so we signal ourselves to avoid deadlock
			SendReadSignal();
		}
		SendWriteSignal();
		// Wait for reader thread
		m_ReadSemaphore->WaitForSignal();
		m_Mutex->Lock();
	}
}

void RingBuffer::SendReadSignal()
{
	if (AtomicCompareExchange(&m_NeedsReadSignal, 0, 1))
	{
		m_ReadSemaphore->Signal();
	}
}

void RingBuffer::SendWriteSignal()
{
	if (AtomicCompareExchange(&m_NeedsWriteSignal, 0, 1))
	{
		m_WriteSemaphore->Signal();
	}
}
void RingBuffer::BufferState::Reset() volatile
{
	bufferPos = 0;
	bufferEnd = 0;
	bufferWraps = 0;
	checkedPos = 0;
	checkedWraps = 0;
}

