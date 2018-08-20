#ifndef THREADED_STREAM_BUFFER_H
#define THREADED_STREAM_BUFFER_H


#include <new> // for placement new


#if defined(__GNUC__) || defined(__SNC__)
#define ALIGN_OF(T) __alignof__(T)
#elif defined(_MSC_VER)
#define ALIGN_OF(T) __alignof(T)
#endif

class Mutex;
class Semaphore;

class RingBuffer 
{
public:
	typedef unsigned long UInt32;
	struct BufferState
	{
		// These should not be size_t, as the GfxDevice may run across processes of different
		// bitness, and the data serialized in the command buffer must match.
		void Reset() volatile;
		volatile UInt32 bufferPos;
		volatile UInt32 bufferEnd;
		volatile UInt32 bufferWraps;
		volatile UInt32 checkedPos;
		volatile UInt32 checkedWraps;
	};

	struct BufferHeader
	{
		BufferState reader;
		BufferState writer;
	};

	typedef unsigned size_t;

	RingBuffer(size_t size);
	~RingBuffer();

	enum
	{
		kDefaultAlignment = 4,
		kDefaultStep = 512
	};

	// Read data from the ringbuffer
	// This function blocks until data new data has arrived in the ringbuffer.
	// It uses semaphores to wait on the producer thread in a efficient way.
	template <class T> const T&	ReadValueType();
	template <class T> T*		ReadArrayType(int count);
	// ReadReleaseData should be called when the data has been read & used completely.
	// At this point the memory will become available to the producer to write into it again.
	void						ReadReleaseData();

	// Write data into the ringbuffer
	template <class T> void		WriteValueType(const T& val);
	template <class T> void		WriteArrayType(const T* vals, int count);
	template <class T> T*		GetWritePointer();
	// WriteSubmitData should be called after data has been completely written and should be made available to the consumer thread to read it.
	// Before WriteSubmitData is called, any data written with WriteValueType can not be read by the consumer.
	void						WriteSubmitData();

	// Ringbuffer Streaming support. This will automatically call WriteSubmitData & ReadReleaseData.
	// It splits the data into smaller chunks (step). So that the size of the ringbuffer can be smaller than the data size passed into this function.
	// The consumer thread will be reading the streaming data while WriteStreamingData is still called on the producer thread.
	void						ReadStreamingData(void* data, size_t size, size_t alignment = kDefaultAlignment, size_t step = kDefaultStep);
	void						WriteStreamingData(const void* data, size_t size, size_t alignment = kDefaultAlignment, size_t step = kDefaultStep);


	// Utility functions
	void*	GetReadDataPointer(size_t size, size_t alignment);
	void*	GetWriteDataPointer(size_t size, size_t alignment);

	size_t	GetDebugReadPosition() const { return m_Reader->bufferPos; }
	size_t	GetDebugWritePosition() const { return m_Writer->bufferPos; }


	// Creation methods
	void	Create(size_t size);
	void	Destroy();

	// 
	// Is there data available to be read
	// typicall this is not used
	bool	HasData() const;
	bool	HasDataToRead() const;

	size_t	GetAllocatedSize() const { return m_BufferSize; }
	size_t	GetCurrentSize() const;
	const void*	GetBuffer() const;


private:
	size_t Align(size_t pos, size_t alignment) const { return (pos + alignment - 1)&~(alignment - 1); }

	void	SetDefaults();

	void	HandleReadOverflow(size_t& dataPos, size_t& dataEnd);
	void	HandleWriteOverflow(size_t& dataPos, size_t& dataEnd);

	void	SendReadSignal();
	void	SendWriteSignal();

	char* m_Buffer;
	size_t m_BufferSize;
	BufferState *m_Reader;
	BufferState *m_Writer;
	BufferHeader m_Header;
	Mutex* m_Mutex;
	Semaphore* m_ReadSemaphore;
	Semaphore* m_WriteSemaphore;
	volatile int m_NeedsReadSignal;
	volatile int m_NeedsWriteSignal;
};

inline bool RingBuffer::HasData() const
{
	return (m_Reader->bufferPos != m_Writer->checkedPos);
}

inline void* RingBuffer::GetReadDataPointer(size_t size, size_t alignment)
{
	size = Align(size, alignment);
	size_t dataPos = Align(m_Reader->bufferPos, alignment);
	size_t dataEnd = dataPos + size;
	if (dataEnd > m_Reader->bufferEnd)
	{
		HandleReadOverflow(dataPos, dataEnd);
	}
	m_Reader->bufferPos = dataEnd;
	
	return &m_Buffer[dataPos];
}

inline void* RingBuffer::GetWriteDataPointer(size_t size, size_t alignment)
{
	size = Align(size, alignment);
	size_t dataPos = Align(m_Writer->bufferPos, alignment);
	size_t dataEnd = dataPos + size;
	if (dataEnd > m_Writer->bufferEnd)
	{
		HandleWriteOverflow(dataPos, dataEnd);
	}
	m_Writer->bufferPos = dataEnd;

	return &m_Buffer[dataPos];
}

template <class T> inline const T& RingBuffer::ReadValueType()
{
	// Read simple data type from queue
	const void* pdata = GetReadDataPointer(sizeof(T), ALIGN_OF(T));
	const T& src = *reinterpret_cast<const T*>(pdata);
	return src;
}

template <class T> inline T* RingBuffer::ReadArrayType(int count)
{
	// Read array of data from queue-
	void* pdata = GetReadDataPointer(count * sizeof(T), ALIGN_OF(T));
	T* src = reinterpret_cast<T*>(pdata);
	return src;
}

template <class T> inline void RingBuffer::WriteValueType(const T& val)
{
	// Write simple data type to queue
	void* pdata = GetWriteDataPointer(sizeof(T), ALIGN_OF(T));
	new (pdata) T(val);
}

template <class T> inline void RingBuffer::WriteArrayType(const T* vals, int count)
{
	// Write array of data to queue
	T* pdata = (T*)GetWriteDataPointer(count * sizeof(T), ALIGN_OF(T));
	for (int i = 0; i < count; i++)
		new (&pdata[i]) T(vals[i]);
}

template <class T> inline T* RingBuffer::GetWritePointer()
{
	// Write simple data type to queue
	void* pdata = GetWriteDataPointer(sizeof(T), ALIGN_OF(T));
	return static_cast<T*>(pdata);
}


#endif
