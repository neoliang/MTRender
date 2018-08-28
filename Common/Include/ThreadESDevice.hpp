//
//  ThreadESDevice.hpp
//  Hello_Triangle
//
//  Created by tencent on 2018/3/22.
//  Copyright © 2018年 Daniel Ginsburg. All rights reserved.
//

#ifndef ThreadESDevice_hpp
#define ThreadESDevice_hpp


#include <queue>
#include "ThreadESDeviceBase.h"
#include "glm/glm.hpp"
namespace RenderEngine {

	template<class T>
	class LockFreeQueue
	{
		const static int MAX_SIZE = 1024;
		T _arr[MAX_SIZE];
		int _in;
		int _out;
		Semaphore _readSem;
		Semaphore _writeSem;
	public:
		LockFreeQueue()
			:_in(0), _out(0)
		{

		}
	public:
		void push(T v)
		{
			int next = (_in + 1) % MAX_SIZE;
			while (next == _out)
			{
				_writeSem.WaitForSignal();
			}
			_arr[_in] = v;
			_in = next;
			_readSem.Signal();
		}
		T Pop()
		{
			while (_out == _in)
			{
				_readSem.WaitForSignal();
			}
			T v = _arr[_out];
			_out = (_out + 1) % MAX_SIZE;
			_writeSem.Signal();
			return v;
		}
	};
	class ThreadESDevice;
	class ThreadDeviceCommand
	{
	public:
		virtual void Execute(ESDevice* device) = 0;
		virtual ~ThreadDeviceCommand() {}
		virtual void OnExecuteEnd(ThreadESDevice* threadDevice) {};
	};

	class CommandQueue {
	public:
		virtual ~CommandQueue() {}
		virtual ThreadDeviceCommand* Pop() = 0;
		virtual void Push(ThreadDeviceCommand* cmd) = 0;
		virtual bool Empty()const = 0;
	};

	class LockFreeCommandQueue : public CommandQueue
	{
	private:
		LockFreeQueue<ThreadDeviceCommand*> _commandQueue;
	public:
		virtual ThreadDeviceCommand* Pop()
		{
			return _commandQueue.Pop();
		}
		virtual void Push(ThreadDeviceCommand* cmd)
		{
			_commandQueue.push(cmd);
		}
		virtual bool Empty()const
		{
			return false;
		}
	};
	class ThreadESDevice : public ThreadESDeviceBase
	{

	protected:
		CommandQueue* _commandQueue;
	public:
		ThreadESDevice(ESContext* context,bool returnResImmediately, CommandQueue* commandQueue = new LockFreeCommandQueue());
		~ThreadESDevice();
		virtual void Clear();
		virtual void UseGPUProgram(GPUProgram* program);
		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader);
		virtual void DeletGPUProgram(GPUProgram* program);
		virtual Texture2D* CreateTexture2D(const TextureData::Ptr& data);
		virtual void DeleteTexture2D(Texture2D* texture);
		virtual void UseTexture2D(Texture2D* texture, unsigned int index);
		virtual void SetClearColor(float r, float g, float b, float alpha);
		virtual void DrawTriangle(std::vector<glm::vec3>& vertices);
		virtual void SetViewPort(int x, int y, int width, int height);
		virtual void BeginRender();
		virtual void Present();
		virtual void AcqiureThreadOwnerShip();
		virtual void ReleaseThreadOwnership();
		virtual VBO* CreateVBO();
		virtual void UpdateVBO(VBO* vbo, const VBOData::Ptr& vboData);
		virtual void DeleteVBO(VBO* vbo);
		virtual void DrawVBO(VBO* vbo);

		virtual void SetGPUProgramParamAsInt(GPUProgramParam* param, int value);

		virtual void SetGPUProgramParamAsFloat(GPUProgramParam* param, float value);

		virtual void SetGPUProgramParamAsMat4(GPUProgramParam* param, const glm::mat4& mat);

		virtual void SetGPUProgramParamAsIntArray(GPUProgramParam* param, const std::vector<int>& values);

		virtual void SetGPUProgramParamAsFloatArray(GPUProgramParam* param, const std::vector<float>& values);

		virtual void SetGPUProgramParamAsMat4Array(GPUProgramParam* param,const std::vector<glm::mat4>& values);

	public: //thread base
		virtual void RunOneThreadCommand();
		virtual void InitThreadGPUProgramParam(ThreadedGPUProgram* program, ThreadedGPUProgramParam* param, const std::string& name);
	};

	class NormalCommandQueue : public CommandQueue
	{
	private:
		std::queue<ThreadDeviceCommand*> _queueImp;
	public:
		virtual ThreadDeviceCommand* Pop()
		{
			auto cmd = _queueImp.front();
			_queueImp.pop();
			return cmd;
		}
		virtual void Push(ThreadDeviceCommand* cmd)
		{
			_queueImp.push(cmd);
		}
		virtual bool Empty()const
		{
			return _queueImp.empty();
		}

	};
	class ThreadDoubleQueueESDevice : public ThreadESDevice
	{
		NormalCommandQueue* _updateQueue;
		NormalCommandQueue* _renderQueue;
		Semaphore _mainThreadSem;
		Semaphore _renderThreadSem;
		volatile bool _suspend;
	public:
		ThreadDoubleQueueESDevice(ESContext* context, bool returnResImmediately)
			:ThreadESDevice(context,returnResImmediately,nullptr)
			,_suspend(false)
		{
			_updateQueue = new	NormalCommandQueue;
			_renderQueue = new NormalCommandQueue;
			_commandQueue = _updateQueue;
		}
		~ThreadDoubleQueueESDevice()
		{
			_commandQueue = nullptr;
			delete _updateQueue;
			delete _renderQueue;
		}

		virtual void BeginRender();
		virtual void Present();
		virtual void RunOneThreadCommand();
	};
}
#endif /* ThreadESDevice_hpp */
