#ifndef ThreadBufferESDevice_H
#define ThreadBufferESDevice_H
#include "ThreadESDeviceBase.h"
#include "RingBuffer.h"
namespace RenderEngine {

	class ThreadBufferESDevice : public ThreadESDeviceBase
	{
	private:
		RingBuffer * _commandBuffer;
		const static unsigned int BUFFER_SIZE = 1024 * 1024;
	public:
		ThreadBufferESDevice(ESContext* context, bool returnResImmediately)
			:ThreadESDeviceBase(context, returnResImmediately) {
			_commandBuffer = new RingBuffer(BUFFER_SIZE);
		}
		~ThreadBufferESDevice() {
			delete _commandBuffer;
		}
	public:
		virtual void Clear();
		virtual void UseGPUProgram(GPUProgram* program);
		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader);
		virtual void DeletGPUProgram(GPUProgram* program);
		virtual Texture2D* CreateTexture2D(int width, int height, const void* data, int dataLen);
		virtual void DeleteTexture2D(Texture2D* texture);
		virtual void UseTexture2D(Texture2D* texture);
		virtual void SetClearColor(float r, float g, float b, float alpha);
		virtual void DrawTriangle(std::vector<glm::vec3>& vertices);
		virtual void SetViewPort(int x, int y, int width, int height);
		virtual void BeginRender();
		virtual void Present();
		virtual void AcqiureThreadOwnerShip();
		virtual void ReleaseThreadOwnership();
		virtual VBO* CreateVBO(std::vector<glm::vec3> vertices,
			std::vector<glm::vec2> uvs,
			std::vector<unsigned short> indices);
		virtual void DeleteVBO(VBO* vbo);
		virtual void DrawVBO(VBO* vbo);

		virtual void SetGPUProgramParamAsInt(GPUProgramParam* param, int value);

		virtual void SetGPUProgramParamAsFloat(GPUProgramParam* param, float value);

		virtual void SetGPUProgramParamAsMat4(GPUProgramParam* param, const glm::mat4& mat);

		virtual void SetGPUProgramParamAsIntArray(GPUProgramParam* param, const std::vector<int>& values);

		virtual void SetGPUProgramParamAsFloatArray(GPUProgramParam* param, const std::vector<float>& values);

		virtual void SetGPUProgramParamAsMat4Array(GPUProgramParam* param, const std::vector<glm::mat4>& values);

	public:		
		virtual void InitThreadGPUProgramParam(ThreadedGPUProgram* program, ThreadedGPUProgramParam* param, const std::string& name);
		virtual void RunOneThreadCommand();
	};
}
#endif
