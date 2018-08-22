//
//  ThreadESDevice.cpp
//  Hello_Triangle
//
//  Created by tencent on 2018/3/22.
//  Copyright © 2018年 Daniel Ginsburg. All rights reserved.
//

#include "ThreadESDevice.hpp"
#include <mutex>
namespace RenderEngine {
	class AutoLock
	{

		std::mutex& _lock;
	public:
		AutoLock(std::mutex& l)
			:_lock(l) {
			//pthread_mutex_lock(&_lock);
		}
		~AutoLock()
		{
			//pthread_mutex_unlock(&_lock);
		}

	};

#define AUTOLOCK
	//#define AUTOLOCK AutoLock lock(_mutex);

	class ClearCMD : public ThreadDeviceCommand
	{
	public:
		virtual void Execute(ESDevice* device)
		{
			device->Clear();
		}
	};

	class SetClearColorCMD : public ThreadDeviceCommand
	{
	private:
		float _r;
		float _g;
		float _b;
		float _a;
	public:
		SetClearColorCMD(float r, float g, float b, float a)
			:_r(r), _g(g), _b(b), _a(a) {};
		void Execute(ESDevice* device)
		{
			device->SetClearColor(_r, _g, _b, _a);
		}
	};
	class DrawTriangleCMD : public ThreadDeviceCommand
	{
	private:
		std::vector<glm::vec3>  _vertices;
	public:
		DrawTriangleCMD(const std::vector<glm::vec3>& vs)
			:_vertices(vs)
		{
		}
		void Execute(ESDevice* device)
		{
			device->DrawTriangle(_vertices);
		}
		~DrawTriangleCMD()
		{
		}
	};
	class SetViewPortCMD : public ThreadDeviceCommand
	{
	private:
		int _x;
		int _y;
		int _width;
		int _height;
	public:
		SetViewPortCMD(int x, int y, int width, int height)
			:_x(x), _y(y), _width(width), _height(height) {}
		void Execute(ESDevice* device)
		{
			device->SetViewPort(_x, _y, _width, _height);
		}
	};
	class PresentCMD : public ThreadDeviceCommand
	{
	public:
		void Execute(ESDevice* device)
		{
			device->Present();
		}
		void OnExecuteEnd(ThreadESDevice* threadDevice);
	};
	class DrawVBOCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedVBO * _vbo;
	public:
		DrawVBOCMD(ThreadedVBO* vbo) :_vbo(vbo) {}
		void Execute(ESDevice* device)
		{
			device->DrawVBO(_vbo->realVbo);
		}
	};
	class DeleteVBOCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedVBO * _vbo;
	public:
		DeleteVBOCMD(ThreadedVBO* vbo) :_vbo(vbo) {}
		void Execute(ESDevice* device)
		{
			device->DeleteVBO(_vbo->realVbo);
			delete _vbo;
		}
	};
	class UpdateVBOCMD : public ThreadDeviceCommand
	{
	private:
		VBOData::Ptr _vboData;
		ThreadedVBO * _vbo;
	public:
		UpdateVBOCMD(const VBOData::Ptr& vboData, ThreadedVBO *vbo)
			:_vboData(vboData), _vbo(vbo) {}
		void Execute(ESDevice* device)
		{
			device->UpdateVBO(_vbo->realVbo, _vboData);
		}
	};
	class CreateVBOCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedVBO * _vbo;
	public:
		CreateVBOCMD(ThreadedVBO *vbo)
			:_vbo(vbo) {}
		void Execute(ESDevice* device)
		{
			_vbo->realVbo = device->CreateVBO();
		}
		void OnExecuteEnd(ThreadESDevice* threadDevice)
		{
			if (threadDevice->IsCreateResInBlockMode())
			{
				threadDevice->Signal(ThreadESDevice::WaitType_CreateVBO);
			}
		}
	};
	class UseTexture2DCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedTexture2D * _texture;
	public:
		UseTexture2DCMD(ThreadedTexture2D* tex) :_texture(tex) {}
		void Execute(ESDevice* device)
		{
			device->UseTexture2D(_texture->realTexture);
		}
	};

	class DeleteTexture2DCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedTexture2D * _texture;
	public:
		DeleteTexture2DCMD(ThreadedTexture2D* tex) :_texture(tex) {}
		void Execute(ESDevice* device)
		{
			device->DeleteTexture2D(_texture->realTexture);
			delete _texture;
		}
	};

	class CreateTexture2DCMD : public ThreadDeviceCommand
	{
	private:
		TextureData::Ptr _data;
		ThreadedTexture2D* _texture;
	public:
		CreateTexture2DCMD(const TextureData::Ptr& data,ThreadedTexture2D* tex)
			: _data(data),_texture(tex)
		{
		}
		void Execute(ESDevice* device)
		{
			_texture->realTexture = device->CreateTexture2D(_data);
		}
		void OnExecuteEnd(ThreadESDevice* threadDevice)
		{
			if (threadDevice->IsCreateResInBlockMode())
			{
				threadDevice->Signal(ThreadESDevice::WaitType_CreateTexture);
			}
		}
	};

	class UseGPUProgramCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedGPUProgram * _program;
	public:
		UseGPUProgramCMD(ThreadedGPUProgram* program) :_program(program) {}
	public:
		void Execute(ESDevice* device)
		{
			device->UseGPUProgram(_program->realProgram);
		}
	};


	class DeleteGPUProgramCMD : public ThreadDeviceCommand
	{
	private:
		ThreadedGPUProgram * _program;
	public:
		DeleteGPUProgramCMD(ThreadedGPUProgram* program) :_program(program) {}
	public:
		void Execute(ESDevice* device)
		{
			device->DeletGPUProgram(_program->realProgram);
			delete _program;
		}
	};
	class CreateGPUProgramCMD : public ThreadDeviceCommand
	{
		const std::string _vsrc;
		const std::string _fsrc;
		ThreadedGPUProgram * _program;
	public:
		CreateGPUProgramCMD(const std::string& vertexShader, const std::string& fragmentShader, ThreadedGPUProgram* program)
			:_vsrc(vertexShader)
			, _fsrc(fragmentShader)
			, _program(program)
		{}
		void Execute(ESDevice* device)
		{
			_program->realProgram = device->CreateGPUProgram(_vsrc, _fsrc);
		}
		void OnExecuteEnd(ThreadESDevice* threadDevice)
		{
			if (threadDevice->IsCreateResInBlockMode())
			{
				threadDevice->Signal(ThreadESDevice::WaitType_CreateShader);
			}
		}
	};
	class AcquireOwnerShipCMD : public ThreadDeviceCommand
	{

		void Execute(ESDevice* device)
		{
			device->AcqiureThreadOwnerShip();
		}
		void OnExecuteEnd(ThreadESDevice* threadDevice);
	};
	class ReleasOwnerShipCMD : public ThreadDeviceCommand
	{
		void Execute(ESDevice* device)
		{
			device->ReleaseThreadOwnership();
		}
		void OnExecuteEnd(ThreadESDevice* threadDevice);
	};
	void AcquireOwnerShipCMD::OnExecuteEnd(ThreadESDevice * threadDevice)
	{
		threadDevice->SignalOnwerShip();
	}

	void ReleasOwnerShipCMD::OnExecuteEnd(ThreadESDevice * threadDevice)
	{
		threadDevice->SignalOnwerShip();
	}
	void ThreadESDevice::Clear()
	{
		if (!_threaded)
		{
			_realDevice->Clear();
			return;
		}
		AUTOLOCK
			_commandQueue.push(new ClearCMD());
	}
	void ThreadESDevice::SetViewPort(int x, int y, int width, int height)
	{
		if (!_threaded)
		{
			_realDevice->SetViewPort(x, y, width, height);
			return;
		}
		AUTOLOCK
			_commandQueue.push(new SetViewPortCMD(x, y, width, height));
	}

	void ThreadESDevice::SetClearColor(float r, float g, float b, float alpha)
	{
		if (!_threaded)
		{
			_realDevice->SetClearColor(r, g, b, alpha);
			return;
		}
		AUTOLOCK
			_commandQueue.push(new SetClearColorCMD(r, g, b, alpha));
	}
	void ThreadESDevice::DrawTriangle(std::vector<glm::vec3>& vertices)
	{
		if (!_threaded)
		{
			_realDevice->DrawTriangle(vertices);
			return;
		}
		AUTOLOCK
			_commandQueue.push(new DrawTriangleCMD(vertices));
	}
	ThreadESDevice::ThreadESDevice(ESContext* context, bool returnResImmediately)
		:ThreadESDeviceBase(context,returnResImmediately)
	{
		esLogMessage("[render] ThreadESDevice");
	}
	ThreadESDevice::~ThreadESDevice()
	{
	}
	void ThreadESDevice::BeginRender()
	{
	}
	void ThreadESDevice::Present()
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
			return;
		}
		{
			AUTOLOCK
				_commandQueue.push(new PresentCMD());
		}
	}
	void RenderEngine::ThreadESDevice::AcqiureThreadOwnerShip()
	{
		esLogMessage("[render] AcqiureThreadOwnerShip %d", (int)_threaded);
		if (!_threaded)
		{
			return;
		}
		_commandQueue.push(new ReleasOwnerShipCMD());
		WaitForOwnerShip();
		_realDevice->AcqiureThreadOwnerShip();
		_threaded = false;
	}
	void RenderEngine::ThreadESDevice::ReleaseThreadOwnership()
	{
		esLogMessage("[render] ReleaseThreadOwnership %d", (int)_threaded);
		if (_threaded)
		{
			return;
		}
		_realDevice->ReleaseThreadOwnership();
		_commandQueue.push(new AcquireOwnerShipCMD());
		WaitForOwnerShip();
		_threaded = true;
	}
	void ThreadESDevice::RunOneThreadCommand()
	{
		ThreadDeviceCommand* cmd = _commandQueue.Pop();
		cmd->Execute(_realDevice);
		cmd->OnExecuteEnd(this);
		delete cmd;
	}


	void PresentCMD::OnExecuteEnd(ThreadESDevice* threadDevice)
	{
		threadDevice->SignalPresent();
	}

	class InitThreadGPUProgramParamCMD : public ThreadDeviceCommand
	{
		ThreadedGPUProgram* _program;
		ThreadedGPUProgramParam* _param;
		const std::string _name;
	public:
		InitThreadGPUProgramParamCMD(ThreadedGPUProgram* program, ThreadedGPUProgramParam* param, const std::string& name)
			:_program(program), _param(param), _name(name) {}
		void Execute(ESDevice* device)
		{
			_param->realParam = device->GetGPUProgramParam(_program->realProgram, _name);
		}
	};
	void ThreadESDevice::InitThreadGPUProgramParam(ThreadedGPUProgram* program, ThreadedGPUProgramParam* param, const std::string& name)
	{
		if (!_threaded)
		{
			param->realParam = _realDevice->GetGPUProgramParam(program->realProgram, name);
		}
		else
		{
			_commandQueue.push(new InitThreadGPUProgramParamCMD(program,param,name));
		}
	}
	void ThreadESDevice::UseGPUProgram(GPUProgram* program)
	{
		ThreadedGPUProgram* threadedP = static_cast<ThreadedGPUProgram*>(program);
		if (!_threaded)
		{
			_realDevice->UseGPUProgram(threadedP->realProgram);
		}
		else
		{
			AUTOLOCK
			_commandQueue.push(new UseGPUProgramCMD(threadedP));
		}		
	}
	void ThreadESDevice::DeletGPUProgram(GPUProgram* program)
	{
		ThreadedGPUProgram* threadedP = static_cast<ThreadedGPUProgram*>(program);
		if (!_threaded)
		{
			_realDevice->DeletGPUProgram(threadedP->realProgram);
			delete threadedP;
		}
		else
		{
			AUTOLOCK
				_commandQueue.push(new DeleteGPUProgramCMD(threadedP));
		}

	}
	GPUProgram* ThreadESDevice::CreateGPUProgram(const std::string &vertexShaderStr, const std::string &fragmentShaderStr)
	{
		ThreadedGPUProgram* program = new ThreadedGPUProgram(this);
		if (!_threaded)
		{
			program->realProgram = _realDevice->CreateGPUProgram(vertexShaderStr, fragmentShaderStr);
		}
		else
		{
			CreateGPUProgramCMD* cmd = new CreateGPUProgramCMD(vertexShaderStr, fragmentShaderStr, program);
			{
				AUTOLOCK
					_commandQueue.push(cmd);
			}
			if (_returnResImmediately)
			{
				WaitForSignal(WaitType_CreateShader);
			}
		}
		return program;
	}
	VBO* ThreadESDevice::CreateVBO()
	{
		auto vbo = new ThreadedVBO();
		if (!_threaded)
		{
			vbo->realVbo = _realDevice->CreateVBO();
		}
		else
		{
			CreateVBOCMD* cmd = new CreateVBOCMD(vbo);
			AUTOLOCK
				_commandQueue.push(cmd);
			if (_returnResImmediately)
			{
				this->WaitForSignal(WaitType_CreateVBO);
			}
		}
		return vbo;
	}
	void ThreadESDevice::UpdateVBO(VBO* vbo, const VBOData::Ptr& vboData)
	{
		ThreadedVBO* threadVbo = static_cast<ThreadedVBO*>(vbo);
		if (!_threaded)
		{
			_realDevice->UpdateVBO(threadVbo->realVbo,vboData);
		}
		else
		{
			UpdateVBOCMD* cmd = new UpdateVBOCMD(vboData, threadVbo);
			_commandQueue.push(cmd);

		}
	}
	void ThreadESDevice::DeleteVBO(VBO* vbo)
	{
		ThreadedVBO* threadedVbo = static_cast<ThreadedVBO*>(vbo);
		if (!_threaded)
		{
			_realDevice->DeleteVBO(threadedVbo->realVbo);
			delete threadedVbo;
		}
		else
		{
			DeleteVBOCMD* cmd = new DeleteVBOCMD(threadedVbo);
			AUTOLOCK
				_commandQueue.push(cmd);
		}
	}
	void ThreadESDevice::DrawVBO(VBO* vbo)
	{
		ThreadedVBO* threadedVbo = static_cast<ThreadedVBO*>(vbo);
		if (!_threaded)
		{
			_realDevice->DrawVBO(threadedVbo->realVbo);
		}
		else
		{
			AUTOLOCK
				_commandQueue.push(new DrawVBOCMD(threadedVbo));
		}
	}
	class SetGPUProgramParamAsIntCMD : public ThreadDeviceCommand
	{
	private:
		GPUProgramParam * _param;
		int _value;
	public:
		SetGPUProgramParamAsIntCMD(GPUProgramParam* param, int value)
			:_param(param), _value(value)
		{}
		void Execute(ESDevice* device)
		{
			device->SetGPUProgramParamAsInt(static_cast<ThreadedGPUProgramParam*>(_param)->realParam, _value);
		}
	};
	void ThreadESDevice::SetGPUProgramParamAsInt(GPUProgramParam* param, int value)
	{
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsInt(param->GetRealParam(), value);
		}
		else
		{
			_commandQueue.push(new SetGPUProgramParamAsIntCMD(param,value));
		}
	}
	class SetGPUProgramParamAsFloatCMD : public ThreadDeviceCommand
	{
	private:
		GPUProgramParam * _param;
		float _value;
	public:
		SetGPUProgramParamAsFloatCMD(GPUProgramParam* param, float value)
			:_param(param), _value(value) 
		{}
		void Execute(ESDevice* device)
		{
			device->SetGPUProgramParamAsFloat(static_cast<ThreadedGPUProgramParam*>(_param)->realParam, _value);
		}
	};
	void ThreadESDevice::SetGPUProgramParamAsFloat(GPUProgramParam* param, float value)
	{
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsFloat(param->GetRealParam(), value);
		}
		else
		{
			_commandQueue.push(new SetGPUProgramParamAsFloatCMD(param, value));
		}
	}
	class SetGPUProgramParamAsMat4CMD : public ThreadDeviceCommand
	{
	private:
		GPUProgramParam * _param;
		glm::mat4 _value;
	public:
		SetGPUProgramParamAsMat4CMD(GPUProgramParam* param, glm::mat4 value)
			:_param(param), _value(value) 
		{}
		void Execute(ESDevice* device)
		{
			device->SetGPUProgramParamAsMat4(static_cast<ThreadedGPUProgramParam*>(_param)->realParam, _value);
		}
	};
	void ThreadESDevice::SetGPUProgramParamAsMat4(GPUProgramParam* param, const glm::mat4& mat)
	{
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsMat4(param->GetRealParam(), mat);
		}
		else
		{
			_commandQueue.push(new SetGPUProgramParamAsMat4CMD(param, mat));
		}
	}
	class SetGPUProgramParamAsIntArrayCMD : public ThreadDeviceCommand
	{
	private:
		GPUProgramParam * _param;
		std::vector<int> _values;
	public:
		SetGPUProgramParamAsIntArrayCMD(GPUProgramParam* param, const std::vector<int>& values)
			:_param(param), _values(values)
		{}
		void Execute(ESDevice* device)
		{
			device->SetGPUProgramParamAsIntArray(static_cast<ThreadedGPUProgramParam*>(_param)->realParam, _values);
		}
	};
	void ThreadESDevice::SetGPUProgramParamAsIntArray(GPUProgramParam* param, const std::vector<int>& values)
	{
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsIntArray(param->GetRealParam(), values);
		}
		else
		{
			_commandQueue.push(new SetGPUProgramParamAsIntArrayCMD(param, values));
		}
	}

	class SetGPUProgramParamAsFloatArrayCMD : public ThreadDeviceCommand
	{
	private:
		GPUProgramParam * _param;
		std::vector<float> _values;
	public:
		SetGPUProgramParamAsFloatArrayCMD(GPUProgramParam* param, const std::vector<float>& values)
			:_param(param), _values(values)
		{}
		void Execute(ESDevice* device)
		{
			device->SetGPUProgramParamAsFloatArray(static_cast<ThreadedGPUProgramParam*>(_param)->realParam, _values);
		}
	};

	void ThreadESDevice::SetGPUProgramParamAsFloatArray(GPUProgramParam* param, const std::vector<float>& values)
	{
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsFloatArray(param->GetRealParam(), values);
		}
		else
		{
			_commandQueue.push(new SetGPUProgramParamAsFloatArrayCMD(param, values));
		}
	}

	class SetGPUProgramParamAsMat4ArrayCMD : public ThreadDeviceCommand
	{
	private:
		GPUProgramParam * _param;
		std::vector<glm::mat4> _values;
	public:
		SetGPUProgramParamAsMat4ArrayCMD(GPUProgramParam* param, const std::vector<glm::mat4>& values)
			:_param(param), _values(values)
		{}
		void Execute(ESDevice* device)
		{
			device->SetGPUProgramParamAsMat4Array(static_cast<ThreadedGPUProgramParam*>(_param)->realParam, _values);
		}
	};

	void ThreadESDevice::SetGPUProgramParamAsMat4Array(GPUProgramParam* param, const std::vector<glm::mat4>& values)
	{
		if (!_threaded)
		{
			_realDevice->SetGPUProgramParamAsMat4Array(param->GetRealParam(), values);
		}
		else
		{
			_commandQueue.push(new SetGPUProgramParamAsMat4ArrayCMD(param, values));
		}
	}

	Texture2D* ThreadESDevice::CreateTexture2D(const TextureData::Ptr& data)
	{
		ThreadedTexture2D* texture = new ThreadedTexture2D();
		if (!_threaded)
		{
			texture->realTexture = _realDevice->CreateTexture2D(data);
		}
		else
		{
			CreateTexture2DCMD* cmd = new CreateTexture2DCMD(data,texture);
			AUTOLOCK
				_commandQueue.push(cmd);
			if (_returnResImmediately)
			{
				WaitForSignal(WaitType_CreateTexture);
			}
				
		}
		return texture;
	}
	void ThreadESDevice::DeleteTexture2D(Texture2D* texture)
	{
		ThreadedTexture2D* threadedText = static_cast<ThreadedTexture2D*>(texture);
		if (!_threaded)
		{
			_realDevice->DeleteTexture2D(threadedText->realTexture);
			delete threadedText;
		}
		else
		{
			DeleteTexture2DCMD* cmd = new DeleteTexture2DCMD(threadedText);
			AUTOLOCK
				_commandQueue.push(cmd);
		}
	}
	void ThreadESDevice::UseTexture2D(Texture2D* texture)
	{
		ThreadedTexture2D* threadedText = static_cast<ThreadedTexture2D*>(texture);
		if (!_threaded)
		{
			_realDevice->UseTexture2D(threadedText->realTexture);
		}
		else
		{
			AUTOLOCK
				_commandQueue.push(new UseTexture2DCMD(threadedText));
		}
	}
}