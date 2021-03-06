#ifndef DemoBase_H
#define DemoBase_H
#include "ESDevice.hpp"
#include "esUtil.h"
#include "Camera.hpp"
class DemoBase
{
public:
	enum DeviceCreateType
	{
		kThreadBuffer,
		kThreadQueue,
		KThreadDoubleQueue,
		kSingleThread,
	};
private:
	RenderEngine::ESDevice* _device;
	RenderEngine::GPUProgram* _program;
	RenderEngine::Texture2D* _texture;
	///
	// Draw a triangle using the shader pair created in Init()
	std::vector<RenderEngine::Mesh::Ptr> _meshes; //= Mesh::Ptr(new Mesh("Cube", 8, 36));
	RenderEngine::Camera::Ptr _camera; //= Camera::Ptr(new Camera);

	bool _returnResImmediately;
	DeviceCreateType _deviceCreateType;
	static DemoBase* gs_demo;
	static bool gs_demoInited;

public:
	DemoBase(bool returnResImmediately, DeviceCreateType type);

	static DemoBase* Instnce();
	virtual void Init();
	virtual void OnCreateDevice(ESContext *esContext);
	virtual void OnDestroyDevice();
	virtual void Update(float dt);
	virtual void Render();


};


#endif
