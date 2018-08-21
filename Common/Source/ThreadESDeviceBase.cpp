#include "ThreadESDeviceBase.h"

namespace RenderEngine {

	GPUProgramParam * ThreadedGPUProgram::GetParam(const std::string & name)
	{
		return _threadDevice->GetGPUProgramParam(this, name);
	}

	GPUProgramParam * ThreadESDeviceBase::GetGPUProgramParam(GPUProgram * program, const std::string & name)
	{
		ThreadedGPUProgramParam* param = new ThreadedGPUProgramParam();
		InitThreadGPUProgramParam(static_cast<ThreadedGPUProgram*>(program), param, name);
		return param;
	}

}