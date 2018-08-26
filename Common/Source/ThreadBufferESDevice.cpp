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
		kGfxCmd_AcqiureThreadOwnerShip,
		kGfxCmd_ReleaseThreadOwnership,
		kGfxCmd_CreateVBO,
		kGfxCmd_UpdateVBO,
		kGfxCmd_DeleteVBO,
		kGfxCmd_DrawVBO,
		kGfxCmd_SetGPUProgramAsInt,
		kGfxCmd_SetGPUProgramAsFloat,
		kGfxCmd_SetGPUProgramAsMat4,
		kGfxCmd_SetGPUProgramAsIntArray,
		kGfxCmd_SetGPUProgramAsFloatArray,
		kGfxCmd_SetGPUProgramAsMat4Array,
		kGfxCmd_InitThreadGPUProgramParam,

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
	struct GfxCmdCreateGPUProgramData
	{
		ThreadedGPUProgram* program;
		unsigned int vSize;
		unsigned int fSize;
	};
	RenderEngine::GPUProgram* ThreadBufferESDevice::CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader)
	{
		ThreadedGPUProgram* program = new ThreadedGPUProgram(this);
		if (!_threaded)
		{
			program->realProgram = _realDevice->CreateGPUProgram(vertexShader, fragmentShader);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_CreateGPUProgram);
			GfxCmdCreateGPUProgramData data{
				program,vertexShader.size(),fragmentShader.size()
			};
			_commandBuffer->WriteValueType(data);
			_commandBuffer->WriteStreamingData(vertexShader.c_str(), vertexShader.size());
			_commandBuffer->WriteStreamingData(fragmentShader.c_str(), fragmentShader.size());
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
	struct GfxCmdCreateTextureData
	{
		ThreadedTexture2D* texture;
		unsigned int width;
		unsigned int height;
		unsigned int dataLen;
	};
	Texture2D* ThreadBufferESDevice::CreateTexture2D(const TextureData::Ptr& data)
	{
		ThreadedTexture2D* texture = new ThreadedTexture2D();
		GfxCmdCreateTextureData cmddata{
			texture,data->width,data->height,data->length
		};
		_commandBuffer->WriteValueType(kGfxCmd_CreateTexture2D);
		_commandBuffer->WriteValueType(cmddata);
		_commandBuffer->WriteStreamingData(data->pixels, data->length);
		return texture;
	}

	void ThreadBufferESDevice::DeleteTexture2D(Texture2D* texture)
	{
		ThreadedTexture2D* threadedText = static_cast<ThreadedTexture2D*>(texture);
		if (!_threaded)
		{
			_realDevice->DeleteTexture2D(threadedText->realTexture);
			delete threadedText;
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_DeleteTexture2D);
			_commandBuffer->WriteValueType(threadedText);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::UseTexture2D(Texture2D* texture, unsigned int index)
	{
		ThreadedTexture2D* threadedText = static_cast<ThreadedTexture2D*>(texture);
		if (!_threaded)
		{
			_realDevice->UseTexture2D(threadedText->realTexture,index);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_UseTexture2D);
			_commandBuffer->WriteValueType(threadedText);
			_commandBuffer->WriteValueType(index);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::SetClearColor(float r, float g, float b, float alpha)
	{
		if (!_threaded)
		{
			_realDevice->SetClearColor(r, g, b, alpha);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetClearColor);
			glm::vec4 data(r, g, b, alpha);
			_commandBuffer->WriteValueType(data);
			_commandBuffer->WriteSubmitData();
		}

	}

	void ThreadBufferESDevice::DrawTriangle(std::vector<glm::vec3>& vertices)
	{
		if (!_threaded)
		{
			_realDevice->DrawTriangle(vertices);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_DrawTriangle);
			unsigned int size = vertices.size() * sizeof(glm::vec3);
			_commandBuffer->WriteValueType(size);
			_commandBuffer->WriteStreamingData(&vertices[0], size);
		}
	}

	void ThreadBufferESDevice::SetViewPort(int x, int y, int width, int height)
	{
		if (!_threaded)
		{
			_realDevice->SetViewPort(x, y, width, height);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetViewPort);
			glm::vec<4, int> data(x,y,width,height);
			_commandBuffer->WriteValueType<glm::vec<4, int>>(data);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::BeginRender()
	{

	}

	void ThreadBufferESDevice::Present()
	{
		if (_isInPresenting)
		{
			WaitForPresent();
		}
		_isInPresenting = true;
		if (!_threaded)
		{
			_realDevice->Present();
			_isInPresenting = false;
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_Present);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::AcqiureThreadOwnerShip()
	{
		esLogMessage("[render] AcqiureThreadOwnerShip %d", (int)_threaded);
		if (!_threaded)
		{
			return;
		}
		_commandBuffer->WriteValueType(kGfxCmd_ReleaseThreadOwnership);
		_commandBuffer->WriteSubmitData();
		WaitForOwnerShip();
		_realDevice->AcqiureThreadOwnerShip();
		_threaded = false;
	}

	void ThreadBufferESDevice::ReleaseThreadOwnership()
	{
		esLogMessage("[render] ReleaseThreadOwnership %d", (int)_threaded);
		if (_threaded)
		{
			return;
		}
		_realDevice->ReleaseThreadOwnership();
		_commandBuffer->WriteValueType(kGfxCmd_AcqiureThreadOwnerShip);
		_commandBuffer->WriteSubmitData();
		WaitForOwnerShip();
		_threaded = true;
	}
	struct GfxCmdUpdateVBOData
	{
		ThreadedVBO* vbo;
		unsigned int verticesCount;
		unsigned int indicesCount;

	};
	VBO* ThreadBufferESDevice::CreateVBO()
	{
		ThreadedVBO* threadvbo = new ThreadedVBO();
		if (!_threaded)
		{
			threadvbo->realVbo =  _realDevice->CreateVBO();
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_CreateVBO);
			_commandBuffer->WriteValueType(threadvbo);
			_commandBuffer->WriteSubmitData();
		}
		return threadvbo;
	}
	void ThreadBufferESDevice::UpdateVBO(VBO* vbo, const VBOData::Ptr& vboData)
	{
		if (!_threaded)
		{
			_realDevice->UpdateVBO(vbo,vboData);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_UpdateVBO);
			GfxCmdUpdateVBOData data{
				(ThreadedVBO*)vbo,vboData->vertices.size(),vboData->indices.size()
			};
			_commandBuffer->WriteValueType(data);
			//BeginProfile("kGfxCmd_UpdateVBO write");
			_commandBuffer->WriteStreamingData(&vboData->vertices[0],data.verticesCount*sizeof(VBOData::Vertex));
			_commandBuffer->WriteStreamingData(&vboData->indices[0],data.indicesCount*sizeof(unsigned short));
			//EndProfile();
		}
	}

	void ThreadBufferESDevice::DeleteVBO(VBO* vbo)
	{
		ThreadedVBO* threadedVbo = static_cast<ThreadedVBO*>(vbo);
		if (!_threaded)
		{
			_realDevice->DeleteVBO(threadedVbo->realVbo);
			delete threadedVbo;
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_DeleteVBO);
			_commandBuffer->WriteValueType(threadedVbo);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::DrawVBO(VBO* vbo)
	{
		ThreadedVBO* threadedVbo = static_cast<ThreadedVBO*>(vbo);
		if (!_threaded)
		{
			_realDevice->DrawVBO(threadedVbo->realVbo);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_DrawVBO);
			_commandBuffer->WriteValueType(threadedVbo);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::SetGPUProgramParamAsInt(GPUProgramParam* param, int value)
	{
		ThreadedGPUProgramParam* threadParam = static_cast<ThreadedGPUProgramParam*>(param);
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsInt(threadParam->realParam,value);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetGPUProgramAsInt);
			_commandBuffer->WriteValueType(threadParam);
			_commandBuffer->WriteValueType(value);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::SetGPUProgramParamAsFloat(GPUProgramParam* param, float value)
	{
		ThreadedGPUProgramParam* threadParam = static_cast<ThreadedGPUProgramParam*>(param);
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsFloat(threadParam->realParam, value);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetGPUProgramAsFloat);
			_commandBuffer->WriteValueType(threadParam);
			_commandBuffer->WriteValueType(value);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::SetGPUProgramParamAsMat4(GPUProgramParam* param, const glm::mat4& mat)
	{
		ThreadedGPUProgramParam* threadParam = static_cast<ThreadedGPUProgramParam*>(param);
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsMat4(threadParam->realParam, mat);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetGPUProgramAsMat4);
			_commandBuffer->WriteValueType(threadParam);
			_commandBuffer->WriteValueType(mat);
			_commandBuffer->WriteSubmitData();
		}
	}

	void ThreadBufferESDevice::SetGPUProgramParamAsIntArray(GPUProgramParam* param, const std::vector<int>& values)
	{
		ThreadedGPUProgramParam* threadParam = static_cast<ThreadedGPUProgramParam*>(param);
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsIntArray(threadParam->realParam, values);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetGPUProgramAsIntArray);
			_commandBuffer->WriteValueType(threadParam);
			unsigned int size = values.size() * sizeof(int);
			_commandBuffer->WriteValueType(size);
			_commandBuffer->WriteStreamingData(&values[0], size);
		}
	}

	void ThreadBufferESDevice::SetGPUProgramParamAsFloatArray(GPUProgramParam* param, const std::vector<float>& values)
	{
		ThreadedGPUProgramParam* threadParam = static_cast<ThreadedGPUProgramParam*>(param);
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsFloatArray(threadParam->realParam, values);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetGPUProgramAsFloatArray);
			_commandBuffer->WriteValueType(threadParam);
			unsigned int size = values.size() * sizeof(float);
			_commandBuffer->WriteValueType(size);
			_commandBuffer->WriteStreamingData(&values[0], size);
		}
	}

	void ThreadBufferESDevice::SetGPUProgramParamAsMat4Array(GPUProgramParam* param, const std::vector<glm::mat4>& values)
	{
		ThreadedGPUProgramParam* threadParam = static_cast<ThreadedGPUProgramParam*>(param);
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsMat4Array(threadParam->realParam, values);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_SetGPUProgramAsMat4Array);
			_commandBuffer->WriteValueType(threadParam);
			unsigned int size = values.size() * sizeof(glm::mat4);
			_commandBuffer->WriteValueType(size);
			_commandBuffer->WriteStreamingData(&values[0][0][0], size);
		}
	}

	void ThreadBufferESDevice::InitThreadGPUProgramParam(ThreadedGPUProgram* program, ThreadedGPUProgramParam* param, const std::string& name)
	{
		if (!_threaded)
		{
			param->realParam = _realDevice->GetGPUProgramParam(program->realProgram, name);
		}
		else
		{
			_commandBuffer->WriteValueType(kGfxCmd_InitThreadGPUProgramParam);
			_commandBuffer->WriteValueType(program);
			_commandBuffer->WriteValueType(param);
			unsigned int size = name.size() * sizeof(char);
			_commandBuffer->WriteValueType(size);
			_commandBuffer->WriteStreamingData(&name[0], size);
		}
	}
	
	void ThreadBufferESDevice::RunOneThreadCommand()
	{
		GfxCommandType cmd = _commandBuffer->ReadValueType<GfxCommandType>();
		switch (cmd)
		{
		case RenderEngine::kGfxCmd_Clear:
		{
			_realDevice->Clear();
			_commandBuffer->ReadReleaseData();
			break;
		}	
		case RenderEngine::kGfxCmd_UseGPUProgram:
		{
			ThreadedGPUProgram* program = _commandBuffer->ReadValueType<ThreadedGPUProgram*>();
			_realDevice->UseGPUProgram(program->realProgram);
			_commandBuffer->ReadReleaseData();
			break;
		}
		case RenderEngine::kGfxCmd_CreateGPUProgram:
		{
			auto data = _commandBuffer->ReadValueType<GfxCmdCreateGPUProgramData>();
			std::string vertexShader, fragmentShader;
			vertexShader.resize(data.vSize);
			fragmentShader.resize(data.fSize);
			_commandBuffer->ReadStreamingData((void*)vertexShader.c_str(), vertexShader.size());	
			_commandBuffer->ReadStreamingData((void*)fragmentShader.c_str(), fragmentShader.size());
			data.program->realProgram = _realDevice->CreateGPUProgram(vertexShader, fragmentShader);
			break;
		}
			
		case RenderEngine::kGfxCmd_DeleteGPUProgram:
		{
			ThreadedGPUProgram* program = _commandBuffer->ReadValueType<ThreadedGPUProgram*>();
			_realDevice->DeletGPUProgram(program->realProgram);
			delete program;
			_commandBuffer->ReadReleaseData();
			break;
		}
		case RenderEngine::kGfxCmd_CreateTexture2D:
		{	
			auto data = _commandBuffer->ReadValueType<GfxCmdCreateTextureData>();			
			char *buff = new char[data.dataLen];
			_commandBuffer->ReadStreamingData(buff, data.dataLen);
			TextureData::Ptr textureData = std::make_shared<TextureData>(buff,data.width,data.height,data.dataLen);
			data.texture->realTexture = _realDevice->CreateTexture2D(textureData);
			break; 
		}

		case RenderEngine::kGfxCmd_DeleteTexture2D:
		{
			auto texture = _commandBuffer->ReadValueType<ThreadedTexture2D*>();
			_realDevice->DeleteTexture2D(texture->realTexture);
			_commandBuffer->ReadReleaseData();
			delete texture;
			break;
		}
		case RenderEngine::kGfxCmd_UseTexture2D:
		{
			auto texture = _commandBuffer->ReadValueType<ThreadedTexture2D*>();
			auto index = _commandBuffer->ReadValueType<unsigned int>();
			_realDevice->UseTexture2D(texture->realTexture,index);
			_commandBuffer->ReadReleaseData();
			break;
		}
			
		case RenderEngine::kGfxCmd_SetClearColor:
		{
			auto data = _commandBuffer->ReadValueType<glm::vec4>();
			_realDevice->SetClearColor(data.r, data.g, data.b, data.a);
			_commandBuffer->ReadReleaseData();
			break;
		}
			
		case RenderEngine::kGfxCmd_DrawTriangle:
		{
			auto size = _commandBuffer->ReadValueType<unsigned int>();
			std::vector<glm::vec3> vertices(size / sizeof(glm::vec3));
			vertices.resize(size / sizeof(glm::vec3));
			_commandBuffer->ReadStreamingData((void*)&vertices[0], size);
			_realDevice->DrawTriangle(vertices);
			break;
		}
			
		case RenderEngine::kGfxCmd_SetViewPort:
		{
			glm::vec<4, int> data = _commandBuffer->ReadValueType<glm::vec<4, int> >();
			_realDevice->SetViewPort(data.x, data.y, data.z, data.w);
			_commandBuffer->ReadReleaseData();
			break;
		}
			
		case RenderEngine::kGfxCmd_Present:
		{
			_realDevice->Present();
			_commandBuffer->ReadReleaseData();
			SignalPresent();
			break;
		}
			
		case RenderEngine::kGfxCmd_AcqiureThreadOwnerShip:
		{	
			_realDevice->AcqiureThreadOwnerShip();
			_commandBuffer->ReadReleaseData();
			break; 
		}
		case RenderEngine::kGfxCmd_ReleaseThreadOwnership:
		{
			_realDevice->ReleaseThreadOwnership();
			_commandBuffer->ReadReleaseData();
			break;
		}
		case RenderEngine::kGfxCmd_CreateVBO:
		{
			ThreadedVBO* threadedVbo = _commandBuffer->ReadValueType<ThreadedVBO*>();
			threadedVbo->realVbo = _realDevice->CreateVBO();
			_commandBuffer->ReadReleaseData();
			break;
		}
		case RenderEngine::kGfxCmd_UpdateVBO:
		{	
			GfxCmdUpdateVBOData data = _commandBuffer->ReadValueType<GfxCmdUpdateVBOData>();
			VBOData::Ptr vboData = std::make_shared<VBOData>();
			BeginProfile("kGfxCmd_UpdateVBO alloc");
			vboData->vertices.resize(data.verticesCount);
			vboData->indices.resize(data.indicesCount);
			EndProfile();
			BeginProfile("kGfxCmd_UpdateVBO read");
			_commandBuffer->ReadStreamingData((void*)&vboData->vertices[0], data.verticesCount * sizeof(VBOData::Vertex));
			_commandBuffer->ReadStreamingData((void*)&vboData->indices[0], data.indicesCount * sizeof(unsigned short));
			EndProfile();
			_realDevice->UpdateVBO(data.vbo->realVbo, vboData);
			vboData.reset();
			break;
		}
		case RenderEngine::kGfxCmd_DeleteVBO:
		{
			ThreadedVBO* threadedVbo = _commandBuffer->ReadValueType<ThreadedVBO*>();
			_realDevice->DeleteVBO(threadedVbo->realVbo);
			delete threadedVbo;
			_commandBuffer->ReadReleaseData();
			break;
		}
		case RenderEngine::kGfxCmd_DrawVBO:
		{
			ThreadedVBO* threadedVbo = _commandBuffer->ReadValueType<ThreadedVBO*>();
			_realDevice->DrawVBO(threadedVbo->realVbo);
			_commandBuffer->ReadReleaseData();
			break;
		}
		case kGfxCmd_SetGPUProgramAsInt:
		{
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			int value = _commandBuffer->ReadValueType<int>();
			_realDevice->SetGPUProgramParamAsInt(threadParam->realParam, value);
			_commandBuffer->ReadReleaseData();
			break;
		}
		case kGfxCmd_SetGPUProgramAsFloat:
		{
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			float value = _commandBuffer->ReadValueType<float>();
			_realDevice->SetGPUProgramParamAsFloat(threadParam->realParam, value);
			_commandBuffer->ReadReleaseData();
			break;
		}

		case kGfxCmd_SetGPUProgramAsMat4:
		{
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			auto value = _commandBuffer->ReadValueType<glm::mat4>();
			_realDevice->SetGPUProgramParamAsMat4(threadParam->realParam, value);
			_commandBuffer->ReadReleaseData();
			break;
		}

		case kGfxCmd_SetGPUProgramAsIntArray:
		{
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			auto size = _commandBuffer->ReadValueType<unsigned int>();
			std::vector<int> values;
			values.resize(size/sizeof(int));
			_commandBuffer->ReadStreamingData(&values[0], size);
			_realDevice->SetGPUProgramParamAsIntArray(threadParam->realParam, values);
			break;
		}

		case kGfxCmd_SetGPUProgramAsFloatArray:
		{
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			auto size = _commandBuffer->ReadValueType<unsigned int>();
			std::vector<float> values;
			values.resize(size/sizeof(float));
			_commandBuffer->ReadStreamingData(&values[0], size);
			_realDevice->SetGPUProgramParamAsFloatArray(threadParam->realParam, values);
			break;
		}

		case kGfxCmd_SetGPUProgramAsMat4Array:
		{
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			auto size = _commandBuffer->ReadValueType<unsigned int>();
			std::vector<glm::mat4> values;
			values.resize(size/sizeof(glm::mat4));
			_commandBuffer->ReadStreamingData(&values[0], size);
			_realDevice->SetGPUProgramParamAsMat4Array(threadParam->realParam, values);
			break;
		}

		case kGfxCmd_InitThreadGPUProgramParam:
		{
			ThreadedGPUProgram* program = _commandBuffer->ReadValueType<ThreadedGPUProgram*>();
			ThreadedGPUProgramParam* threadParam = _commandBuffer->ReadValueType<ThreadedGPUProgramParam*>();
			auto size = _commandBuffer->ReadValueType<unsigned int>();
			std::string name;
			name.resize(size/sizeof(char));
			_commandBuffer->ReadStreamingData((void*)name.c_str(), size);
			threadParam->realParam = _realDevice->GetGPUProgramParam(program->realProgram, name);
			break;
		}

		default:
			assert(false);
			break;
		}
	}



}