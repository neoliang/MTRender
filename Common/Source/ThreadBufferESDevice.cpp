#include "ThreadBufferESDevice.h"
namespace RenderEngine
{
	enum GfxCommandType
	{
		kGfxCmd_Unused = 10000,

		kGfxCmd_Clear,
		kGfxCmd_UseGPUProgram,
		kGfxCmd_CreateGPUProgram,
		kGfxCmd_DeleteGPUProgram,
		kGfxCmd_CreateTexture2D,
		kGfxCmd_DeleteTexture2D,
		kGfxCmd_UseTexture2D,
		kGfxCmd_SetClearColor,
		kGfxCmd_DrawTriangle,
		kGfxCmd_SetViewPort,
		kGfxCmd_Present,
		kGfxCmd_Render,
		kGfxCmd_AcqiureThreadOwnerShip,
		kGfxCmd_ReleaseThreadOwnership,
		kGfxCmd_CreateVBO,
		kGfxCmd_DeleteVBO,
		kGfxCmd_DrawVBO,


		kGfxCmd_Count
	};

	void ThreadBufferESDevice::Clear()
	{
		if (!_threaded)
		{
			_realDevice->Clear();
			return;
		}
		_commandBuffer->WriteValueType(kGfxCmd_Clear);
		_commandBuffer->WriteSubmitData();
	}

	void ThreadBufferESDevice::UseGPUProgram(GPUProgram* program)
	{
		ThreadedGPUProgram* threadedP = static_cast<ThreadedGPUProgram*>(program);
		if (!_threaded)
		{
			_realDevice->UseGPUProgram(threadedP->realProgram);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_UseGPUProgram);
			_commandBuffer->WriteValueType<ThreadedGPUProgram*>(threadedP);
			_commandBuffer->WriteSubmitData();
		}
	}

	RenderEngine::GPUProgram* ThreadBufferESDevice::CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader)
	{
		ThreadedGPUProgram* program = new ThreadedGPUProgram();
		if (!_threaded)
		{
			program->realProgram = _realDevice->CreateGPUProgram(vertexShader, fragmentShader);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_CreateGPUProgram);
			_commandBuffer->WriteStreamingData(vertexShader.c_str(), vertexShader.size());
			_commandBuffer->WriteStreamingData(fragmentShader.c_str(), fragmentShader.size());
			_commandBuffer->WriteValueType<ThreadedGPUProgram*>(program);
			_commandBuffer->WriteSubmitData();
			if (_returnResImmediately)
			{
				WaitForSignal(WaitType_CreateShader);
			}
		}
		return program;
	}

	void ThreadBufferESDevice::DeletGPUProgram(GPUProgram* program)
	{
		ThreadedGPUProgram* threadedP = static_cast<ThreadedGPUProgram*>(program);
		if (!_threaded)
		{
			_realDevice->DeletGPUProgram(threadedP->realProgram);
			delete threadedP;
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_DeleteGPUProgram);
			_commandBuffer->WriteValueType<ThreadedGPUProgram*>(threadedP);
			_commandBuffer->WriteSubmitData();
		}
	}

	Texture2D* ThreadBufferESDevice::CreateTexture2D(int width, int height, const void* data, int dataLen)
	{
		ThreadedTexture2D* texture = new ThreadedTexture2D();
		if (!_threaded)
		{
			texture->realTexture = _realDevice->CreateTexture2D(width, height, data,dataLen);
		}
		else
		{

			if (_returnResImmediately)
			{
				WaitForSignal(WaitType_CreateTexture);
			}

		}
		return texture;
	}

	void ThreadBufferESDevice::DeleteTexture2D(Texture2D* texture)
	{

	}

	void ThreadBufferESDevice::UseTexture2D(Texture2D* texture)
	{

	}

	void ThreadBufferESDevice::SetClearColor(float r, float g, float b, float alpha)
	{

	}

	void ThreadBufferESDevice::DrawTriangle(std::vector<glm::vec3>& vertices)
	{

	}

	void ThreadBufferESDevice::SetViewPort(int x, int y, int width, int height)
	{

	}

	void ThreadBufferESDevice::BeginRender()
	{

	}

	void ThreadBufferESDevice::Present()
	{

	}

	void ThreadBufferESDevice::Render(Camera::Ptr camer, const std::vector<Mesh::Ptr>& mesh)
	{

	}

	void ThreadBufferESDevice::AcqiureThreadOwnerShip()
	{

	}

	void ThreadBufferESDevice::ReleaseThreadOwnership()
	{

	}

	RenderEngine::VBO* ThreadBufferESDevice::CreateVBO(std::vector<glm::vec3> vertices, std::vector<glm::vec2> uvs, std::vector<unsigned short> indices)
	{
		return nullptr;
	}

	void ThreadBufferESDevice::DeleteVBO(VBO* vbo)
	{

	}

	void ThreadBufferESDevice::DrawVBO(VBO* vbo)
	{

	}

	void ThreadBufferESDevice::RunOneThreadCommand()
	{

	}

}