#ifndef RingBuffer_h
#define RingBuffer_h
#include "esUtil.h"
#include "PlatformSemaphore.h"
#include <vector>
#include <mutex>

#define min(a,b)            (((a) < (b)) ? (a) : (b))
class RingBuffer
{
	const static int MAX_SIZE = 1024;
	char _bufer[MAX_SIZE];
	unsigned long  _readPos;
	unsigned long  _writePos;
	Semaphore _readSem;
	Semaphore _writeSem;
	std::vector<char> _tempbufer;

#define  currentCanBeReadSzie ((MAX_SIZE + _in - _out)&(MAX_SIZE - 1))
#define currentCanbeWriteSize ( MAX_SIZE + _out - _in)
private:
	void ReadDataPointer(char* data, unsigned int size, unsigned int alignment)
	{
		
		unsigned int StepSize = Align(size, alignment);
		unsigned int nextPos = (_readPos + StepSize)&(MAX_SIZE - 1);
		unsigned int tempWritePos = _writePos;
		while (_readPos == tempWritePos
			|| (_readPos > tempWritePos && tempWritePos < nextPos && nextPos < _readPos)
			|| (_readPos < tempWritePos && (nextPos < _readPos || nextPos > tempWritePos)))
		{		
			_readSem.WaitForSignal();
			tempWritePos = _writePos;
		}
		//_lock.lock();
		unsigned int l = min(size, MAX_SIZE - (_readPos & (MAX_SIZE - 1)));
		long temp;
		__asm xchg temp, eax;
		//barrier
		memcpy(data, _bufer + (_readPos & (MAX_SIZE - 1)), l);
		memcpy(data + l, _bufer, size - l);
		__asm xchg temp, eax;
		//barrier
		_readPos = (_readPos + StepSize) & (MAX_SIZE - 1);
		//_lock.unlock();
		_writeSem.Signal();
		
	}

	void WriteDataPointer(const char* data, unsigned int size, unsigned int alignment)
	{
		unsigned int StepSize = Align(size, alignment);
		unsigned int writePos = _writePos;
		unsigned int readPos = _readPos;
		unsigned int nextPos = (writePos + StepSize) & (MAX_SIZE - 1);
		while ( nextPos == readPos 
			|| (writePos < readPos && nextPos >= readPos)
			|| (writePos > readPos && nextPos >= readPos && nextPos < writePos))
		{
			_writeSem.WaitForSignal();
			readPos = _readPos;
		}
		//_lock.lock();
		unsigned int l = min(size, MAX_SIZE - (_writePos  & (MAX_SIZE - 1)));
		//barrier
		memcpy(_bufer + (_writePos & (MAX_SIZE - 1)), data, l);
		//放在buffer的最开始
		memcpy(_bufer, (data + l), size - l);
		//barrier
		
		_writePos = (_writePos + StepSize) & (MAX_SIZE - 1);
		//_lock.unlock();
		_readSem.Signal();
	}
public:
	static unsigned int Align(unsigned int pos, unsigned int alignment) { return (pos + alignment - 1)&~(alignment - 1); }
public:
	RingBuffer()
		:_readPos(0), _writePos(0)
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
		WriteDataPointer((const char*)&v, size, kDefaultAlignment);
	}
	template<class T>  const T& Read()
	{
		auto size = sizeof(T);
		_tempbufer.resize(size);
		ReadDataPointer((char*)&_tempbufer[0], size, kDefaultAlignment);
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
