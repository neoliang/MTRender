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

		}
	}

	RenderEngine::GPUProgram* ThreadBufferESDevice::CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader)
	{
		return nullptr;
	}

	void ThreadBufferESDevice::DeletGPUProgram(GPUProgram* program)
	{

	}

	RenderEngine::Texture2D* ThreadBufferESDevice::CreateTexture2D(int width, int height, const void* data)
	{
		return nullptr;
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