#ifndef ThreadESDeviceBase_h
#define ThreadESDeviceBase_h
#include "ESDevice.hpp"
#include <thread>
#include "esUtil.h"
#include "PlatformSemaphore.h"
namespace RenderEngine {

	class ThreadESDeviceBase;

	class ThreadedGPUProgram : public GPUProgram
	{
		friend class DeleteGPUProgramCMD;
		friend class ThreadESDevice;
		friend class ThreadBufferESDevice;
	public:
		ThreadESDeviceBase * _threadDevice;
		GPUProgram * realProgram;
		GPUProgram* GetRealGUPProgram()
		{
			return realProgram;
		}
		GPUProgramParam* GetParam(const std::string& name);
	protected:
		~ThreadedGPUProgram() {}
		ThreadedGPUProgram(ThreadESDeviceBase* threadDevice) :_threadDevice(threadDevice), realProgram(NULL) {}
	};

	class ThreadedTexture2D : public Texture2D
	{
		friend class DeleteTexture2DCMD;
		friend class ThreadESDevice;
		friend class ThreadBufferESDevice;
	public:
		Texture2D * realTexture;	
		Texture2D* GetRealTexture2D()
		{
			return realTexture;
		}
	protected:
		~ThreadedTexture2D() {}
		ThreadedTexture2D() :realTexture(NULL) {}
	};

	class ThreadedVBO : public VBO
	{
		friend class ThreadESDevice;
		friend class DeleteVBOCMD;
		friend class ThreadBufferESDevice;
	public:
		VBO * realVbo;
	protected:
		~ThreadedVBO() {}
		ThreadedVBO() {}
	public:
		virtual VBO* GetRealVBO()
		{
			return realVbo;
		}
	};

	class ThreadedGPUProgramParam : public GPUProgramParam
	{
		friend class ThreadESDeviceBase;
	protected:
		ThreadedGPUProgramParam() {}
		~ThreadedGPUProgramParam() {}

	public:
		GPUProgramParam * realParam;

		virtual GPUProgramParam* GetRealParam()
		{
			return realParam;
		}
	};

	class ThreadESDeviceBase : public ESDevice
	{
	public:
		enum WaitType
		{
			WaitType_Common,
			WaitType_OnwerShip,
			WaitType_Present,
			WaitType_CreateShader,
			WaitType_CreateVBO,
			WaitType_CreateTexture,

			WaitType_Max
		};
	private:
		std::thread _thread;
		bool _quit;
		Semaphore _waitSem[WaitType_Max];
	protected:
		bool  _threaded;
		bool _isInPresenting;
		bool _returnResImmediately;	//是否立即返回资源创建
	protected:
		ESDevice * _realDevice;
		virtual void RunOneThreadCommand()=0;
	public:
		ThreadESDeviceBase(ESContext* context, bool returnResImmediately)
			:_returnResImmediately(returnResImmediately)
			, _threaded(false)
			, _quit(false)
			, _isInPresenting(false)
		{
			esLogMessage("[render] ThreadESDevice");
			_realDevice = new ESDeviceImp(context);
		}
		virtual bool CreateWindow1(const std::string& title, int width, int height, int flags)
		{
			return _realDevice->CreateWindow1(title, width, height, flags);
		}
		virtual void Cleanup()
		{
			_quit = true;
			_thread.join();
			delete _realDevice;
		}
		virtual int GetScreenWidth() { return _realDevice->GetScreenWidth(); }
		virtual int GetScreenHeigt() { return _realDevice->GetScreenHeigt(); }

		virtual GPUProgramParam* GetGPUProgramParam(GPUProgram* program, const std::string& name);

		virtual void InitThreadGPUProgramParam(ThreadedGPUProgram* program, ThreadedGPUProgramParam* param, const std::string& name) = 0;
	public:
		bool IsCreateResInBlockMode()const
		{
			return _returnResImmediately;
		}
		void WaitForSignal(WaitType waitType = WaitType_Common) {
			_waitSem[waitType].WaitForSignal();
		}
		void Signal(WaitType waitType = WaitType_Common) {
			_waitSem[waitType].Signal();
		}
		void WaitForPresent()
		{
			WaitForSignal(WaitType_Present);
		}
		void SignalPresent()
		{
			Signal(WaitType_Present);
			_isInPresenting = false;
		}
		void WaitForOwnerShip()
		{
			WaitForSignal(WaitType_OnwerShip);
		}
		void SignalOnwerShip()
		{
			Signal(WaitType_OnwerShip);
		}
	private:
		static void* _Run(void* data)
		{
			esLogMessage("[render] _Run(void* data)");
			ThreadESDeviceBase* self = (ThreadESDeviceBase*)data;
			self->_RunCommand();
			esLogMessage("[render] _Run(void* data) end");
			return 0;
		}
		virtual void _RunCommand()
		{
			esLogMessage("[render] __RunCommand()");
			_threaded = true;
			_realDevice->AcqiureThreadOwnerShip();
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			Signal();
			while (!_quit) {
				//usleep(10);
				if (_threaded) {
					RunOneThreadCommand();
				}
			}
			esLogMessage("[render] __RunCommand() end");
		}
	public:
		void Run()
		{
			esLogMessage("[render] ThreadESDevice::Run()");
			_thread = std::thread(&ThreadESDeviceBase::_Run, (void*)this);
			//_realDevice->AcqiureThreadOwnerShip();
			this->WaitForSignal();
		}
	};
}
#endif
