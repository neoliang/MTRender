#ifndef DemoBase_H
#define DemoBase_H
#include "ESDevice.hpp"
#include "esUtil.h"

class DemoBase
{
	RenderEngine::ESDevice* g_device;
	RenderEngine::GPUProgram* g_program;
	RenderEngine::Texture2D* g_texture;
	///
	// Draw a triangle using the shader pair created in Init()
	std::vector<RenderEngine::Mesh::Ptr> meshes; //= Mesh::Ptr(new Mesh("Cube", 8, 36));
	RenderEngine::Camera::Ptr camera; //= Camera::Ptr(new Camera);

	bool _multiThreaded;
	bool _returnResImmediately;
	static DemoBase* gs_demo;
	static bool gs_demoInited;
public:
	DemoBase(bool multiThreaded, bool returnResImmediately);

	static DemoBase* Instnce();
	virtual void Init();
	virtual void OnCreateDevice(ESContext *esContext);
	virtual void OnDestroyDevice();
	virtual void Update(float dt);
	virtual void Render();


};


#endif
