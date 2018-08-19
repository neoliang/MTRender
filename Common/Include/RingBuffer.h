#ifndef RingBuffer_h
#define RingBuffer_h
#include "esUtil.h"
#include "PlatformSemaphore.h"
#include <vector>


#define min(a,b)            (((a) < (b)) ? (a) : (b))
class RingBuffer
{
	const static int MAX_SIZE = 1024;
	char _bufer[MAX_SIZE];
	unsigned long long _in;
	unsigned long long _out;
	Semaphore _readSem;
	Semaphore _writeSem;
	std::vector<char> _tempbufer;

private:
	void ReadDataPointer(char* data, unsigned int size, unsigned int alignment)
	{
		unsigned int StepSize = Align(size, alignment);
		if (StepSize <= _in - _out)
		{
			unsigned int l = min(size, MAX_SIZE - (_out & (MAX_SIZE - 1)));
			//barrier
			memcpy(data, _bufer + (_out & (MAX_SIZE - 1)), l);
			memcpy(data + l, _bufer, size - l);
			//barrier
			_out += StepSize;
			_writeSem.Signal();
		}
		else
		{
			unsigned int readSize = size - (_in - _out);
			unsigned int readStepSize = Align(readSize, alignment);
			while (readStepSize > _in - _out)
			{
				_readSem.WaitForSignal();
			}
			ReadDataPointer(data, readSize,alignment);
			if (size - readSize > 0)
			{
				ReadDataPointer(data + readSize, size - readSize, alignment);
			}
		}
	}
	void WriteDataPointer(const char* data, unsigned int size, unsigned int alignment)
	{
		unsigned int StepSize = Align(size, alignment);
		if (StepSize <= MAX_SIZE - _in + _out)
		{
			unsigned int l = min(size, MAX_SIZE - (_in  & (MAX_SIZE - 1)));
			//barrier
			memcpy(_bufer + (_in & (MAX_SIZE - 1)), data, l);
			memcpy(_bufer, (data + l), size - l);
			//barrier
			_in = _in + StepSize;
			_readSem.Signal();
		}
		else
		{
			unsigned int writeSize = size - (MAX_SIZE - _in + _out);
			unsigned int writeStepSize = Align(writeSize, alignment);
			while (writeSize > MAX_SIZE - _in + _out)
			{
				_writeSem.WaitForSignal();
			}
			WriteDataPointer(data, writeSize,alignment);
			if (size - writeSize > 0)
			{
				WriteDataPointer(data + writeSize, size - writeSize, alignment);
			}
		}
	}
public:
	static unsigned int Align(unsigned int pos, unsigned int alignment)  { return (pos + alignment - 1)&~(alignment - 1); }
public:
	RingBuffer()
		:_in(0), _out(0)
	{
		_tempbufer.reserve(1024);
	}
public:

	enum
	{
		kDefaultAlignment = 4,
		kDefaultStep = 512
	};

	template<class T>  void Write(const T& v)
	{
		auto size = sizeof(T);
		WriteDataPointer((const char*)&v,size, kDefaultAlignment);
	}
	template<class T>  const T& Read()
	{
		auto size = sizeof(T);
		_tempbufer.resize(size);
		ReadDataPointer((char*)&_tempbufer[0],size, kDefaultAlignment);
		return *reinterpret_cast<const T*>(&_tempbufer[0]);
	}


	void ReadStreamingData(void* data, unsigned int size, unsigned int alignment = kDefaultAlignment, unsigned int step = kDefaultStep)
	{
		char* dest = (char*)data;
		for (unsigned int offset = 0; offset < size; offset += step)
		{
			unsigned int bytes = min(size - offset, step);
			ReadDataPointer(dest, bytes, alignment);
			dest += step;
		}
	}
	void WriteStreamingData(const void* data, unsigned int size, unsigned int alignment = kDefaultAlignment, size_t step = kDefaultStep)
	{
		const char* src = (const char*)data;
		for (unsigned int offset = 0; offset < size; offset += step)
		{
			unsigned int bytes = min(size - offset, step);
			WriteDataPointer(src, bytes, alignment);
			src += step;
		}
	}
};

#endif
