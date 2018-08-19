#ifndef RingBuffer_h
#define RingBuffer_h
#include "esUtil.h"
#include "PlatformSemaphore.h"
#include <vector>

#if defined(__GNUC__) || defined(__SNC__)
#define ALIGN_OF(T) __alignof__(T)
#define ALIGN_TYPE(val) __attribute__((aligned(val)))
#define FORCE_INLINE inline __attribute__ ((always_inline))
#elif defined(_MSC_VER)
#define ALIGN_OF(T) __alignof(T)
#define ALIGN_TYPE(val) __declspec(align(val))
#define FORCE_INLINE __forceinline
#else
#define ALIGN_TYPE(size)
#define FORCE_INLINE inline
#endif

#define min(a,b)            (((a) < (b)) ? (a) : (b))
class RingBuffer
{
	const static int MAX_SIZE = 1024 * 1024;
	char _bufer[MAX_SIZE];
	unsigned long long _in;
	unsigned long long _out;
	Semaphore _readSem;
	Semaphore _writeSem;
	std::vector<char> _tempbufer;

private:
	FORCE_INLINE void ReadDataPointer(char* data, unsigned int size)
	{
		if (size <= _in - _out)
		{
			unsigned int l = min(size, MAX_SIZE - (_out & (MAX_SIZE - 1)));
			//barrier
			memcpy(data, _bufer + (_out & (MAX_SIZE - 1)), l);
			memcpy(data + l, _bufer, size - l);
			//barrier
			_out += size;
			_writeSem.Signal();
		}
		else
		{
			unsigned int readSize = size - (_in - _out);
			ReadDataPointer(data, readSize);
			_readSem.WaitForSignal();
			ReadDataPointer(data + readSize, size - readSize);
		}
	}
	FORCE_INLINE void WriteDataPointer(const char* data, unsigned int size)
	{
		if (size <= MAX_SIZE - _in + _out)
		{
			unsigned int l = min(size, MAX_SIZE - (_in  & (MAX_SIZE - 1)));
			//barrier
			memcpy(_bufer + (_in & (MAX_SIZE - 1)), data, l);
			memcpy(_bufer, (data + l), size - l);
			//barrier
			_in = _in + size;
			_readSem.Signal();
		}
		else
		{
			unsigned int writeSize = size - (MAX_SIZE - _in + _out);
			WriteDataPointer(data, writeSize);
			_writeSem.WaitForSignal();
			WriteDataPointer(data + writeSize, size - writeSize);
		}
	}
	FORCE_INLINE size_t Align(size_t pos, size_t alignment) const { return (pos + alignment - 1)&~(alignment - 1); }
public:
	RingBuffer()
		:_in(0), _out(0)
	{
		_tempbufer.reserve(1024);
	}
public:
	template<class T> FORCE_INLINE void Write(const T& v)
	{
		auto size = Align(sizeof(T), ALIGN_OF(T));
		WriteDataPointer((const char*)&v,size);
	}
	template<class T> FORCE_INLINE const T& Read()
	{
		auto size = Align(sizeof(T), ALIGN_OF(T));
		_tempbufer.resize(size);
		return ReadDataPointer<T>((char*)&_tempbufer[0],size);
		return *reinterpret_cast<const T*>(&_tempbufer[0]);
	}

	enum
	{
		kDefaultAlignment = 4,
		kDefaultStep = 4096
	};

	void ReadStreamingData(void* data, unsigned int size, unsigned int alignment = kDefaultAlignment, unsigned int step = kDefaultStep)
	{
		char* dest = (char*)data;
		for (unsigned int offset = 0; offset < size; offset += step)
		{
			unsigned int bytes = min(size - offset, step);
			ReadDataPointer(dest, Align(bytes, alignment));
			dest += step;
		}
	}
	void WriteStreamingData(const void* data, unsigned int size, unsigned int alignment = kDefaultAlignment, size_t step = kDefaultStep)
	{
		const char* src = (const char*)data;
		for (unsigned int offset = 0; offset < size; offset += step)
		{
			unsigned int bytes = min(size - offset, step);
			WriteDataPointer(src, Align(bytes, alignment));
			src += step;
		}
	}
};

#endif
