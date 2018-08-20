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

	class ThreadESDevice : public ThreadESDeviceBase
	{

	private:
		LockFreeQueue<ThreadDeviceCommand*> _commandQueue;
	public:
		ThreadESDevice(ESContext* context,bool returnResImmediately);
		~ThreadESDevice();
		virtual void Clear();
		virtual void UseGPUProgram(GPUProgram* program);
		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader);
		virtual void DeletGPUProgram(GPUProgram* program);
		virtual Texture2D* CreateTexture2D(int width, int height, const void* data);
		virtual void DeleteTexture2D(Texture2D* texture);
		virtual void UseTexture2D(Texture2D* texture);
		virtual void SetClearColor(float r, float g, float b, float alpha);
		virtual void DrawTriangle(std::vector<glm::vec3>& vertices);
		virtual void SetViewPort(int x, int y, int width, int height);
		virtual void BeginRender();
		virtual void Present();
		virtual void Render(Camera::Ptr camer, const std::vector<Mesh::Ptr>& mesh);
		virtual void AcqiureThreadOwnerShip();
		virtual void ReleaseThreadOwnership();
		virtual VBO* CreateVBO(std::vector<glm::vec3> vertices,
			std::vector<glm::vec2> uvs,
			std::vector<unsigned short> indices);
		virtual void DeleteVBO(VBO* vbo);
		virtual void DrawVBO(VBO* vbo);
	public:
		virtual void RunOneThreadCommand();
	};
}
#endif /* ThreadESDevice_hpp */
