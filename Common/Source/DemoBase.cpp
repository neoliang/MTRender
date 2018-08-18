#include "DemoBase.h"
#include "esUtil.h"
#include "ESDevice.hpp"
#include "ThreadESDevice.hpp"
#include <cmath>
#include <thread>
#include <iostream>
#include "glm/glm.hpp"
using namespace std;
using namespace RenderEngine;
using namespace  glm;
#ifdef _WIN32
const float M_PI = 3.1415926f;
#endif

const std::string vShaderStr =
"#version 300 es                          \n"
"layout(location = 0) in vec4 vPosition;  \n"
"layout(location = 1) in vec2 a_texCoord; \n"
"out vec2 v_texCoord;                     \n"
"	uniform mat4 MVP;                     \n"
"void main()                              \n"
"{                                        \n"
"   gl_Position = MVP * vPosition;		  \n"
"   v_texCoord = a_texCoord;              \n"
"}                                        \n";

const std::string fShaderStr =
"#version 300 es                              \n"
"precision mediump float;                     \n"
"in vec2 v_texCoord;                          \n"
"layout(location = 0) out vec4 fragColor;                          \n"
"uniform sampler2D baseTex;                 \n"
"void main()                                  \n"
"{                                            \n"
"   fragColor = texture( baseTex, v_texCoord );  \n"
"}                                            \n";


DemoBase*DemoBase::gs_demo(nullptr);
bool DemoBase::gs_demoInited(false);

DemoBase::DemoBase(bool multiThreaded, bool returnResImmediately)
:_multiThreaded(multiThreaded)
,_returnResImmediately(returnResImmediately)
{
	gs_demo = this;
}

DemoBase* DemoBase::Instnce()
{
	if (!gs_demoInited)
	{
		gs_demo->Init();
		gs_demoInited = true;
	}
	return gs_demo;
}

void DemoBase::Init()
{
	_meshes = Mesh::LoadMeshFromFile("monkey.babylon");
	auto tM = _meshes[0];
	for (int i = 0; i < 40; ++i)
	{
		auto m = std::make_shared<Mesh>("cube", tM->vertices.size(), tM->indices.size());
		m->position = glm::vec3((rand() % 10)-5, (rand() % 10)-5, (rand() % 10) - 5);
		_meshes.push_back(m);
	}
	_camera = Camera::Ptr(new Camera);
	_camera->position = vec3(0, 0, 12.0f);
	_camera->target = vec3(0, 0, 0);
}

void DemoBase::OnCreateDevice(ESContext *esContext)
{
#ifdef __APPLE__
	g_device = new ESDeviceImp(esContext);
#else
	if (_multiThreaded)
	{
		_device = new ThreadESDevice(esContext, _returnResImmediately);
	}
	else
	{
		_device = new ESDeviceImp(esContext);
	}
#endif
	_device->CreateWindow1("Hello Triangle", 480, 320, ES_WINDOW_RGB | ES_WINDOW_DEPTH | ES_WINDOW_ALPHA);
	if (!_multiThreaded)
	{
		_device->AcqiureThreadOwnerShip();
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
	}
	else
	{
#ifndef __APPLE__
		((ThreadESDevice*)_device)->Run();
#endif
	}
	_device->SetClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	int width,
		height;

	char *buffer = esLoadTGA(esContext->platformData, "basemap.tga", &width, &height);
	_texture = _device->CreateTexture2D(width, height, buffer);
	_program = _device->CreateGPUProgram(vShaderStr, fShaderStr);
	for (auto mesh : _meshes)
	{
		mesh->vbo = _device->CreateVBO(_meshes[0]->vertices, _meshes[0]->uvs, _meshes[0]->indices);
	}
}

void DemoBase::OnDestroyDevice()
{
	_device->DeletGPUProgram(_program);
	_device->DeleteTexture2D(_texture);
	for (auto mesh : _meshes)
	{
		_device->DeleteVBO(mesh->vbo);
	}
	_device->Cleanup();
	delete _device;
	_device = nullptr;
}

static double g_accTime = 0;
static unsigned g_accCount = 0;
static float g_fps = 0.0f;
void DemoBase::Update(float dt)
{
	++g_accCount;
	g_accTime += dt;
	float avgFps = 1.0f / (g_accTime / g_accCount);


	float newFPs = 1.0f / dt;
	float delta = newFPs - g_fps;
	g_fps = newFPs;
	for (auto mesh : _meshes)
	{
		mesh->rotation = vec3(mesh->rotation.x + dt * 0.5f, mesh->rotation.y + dt * 0.2f, mesh->rotation.z);
	}
	esLogMessage("Update avgfps: %f currentFPs: %f delta: %f\n", avgFps, newFPs, delta);
	ESSleep(0.02f);
}

void DemoBase::Render()
{
	//BeginProfile("g_device->BeginRender()");
	_device->BeginRender();
	//EndProfile();
	//g_device->SetViewPort(0, 0, esContext->width, esContext->height);

	_device->Clear();
	_device->UseTexture2D(_texture);
	auto program = _device->CreateGPUProgram(vShaderStr, fShaderStr);
	_device->UseGPUProgram(program);
	_device->DeletGPUProgram(program);
	//for (int i = 0; i < 40; ++i)
	{
		_device->Render(_camera, _meshes);
	}
	_device->Present();
}




void Draw(ESContext *esContext)
{
	DemoBase::Instnce()->Render();
}

void Shutdown(ESContext *esContext)
{
	esLogMessage("Shutdown");
	DemoBase::Instnce()->OnDestroyDevice();
}

void Update(ESContext* esContext, float dt)
{
	DemoBase::Instnce()->Update(dt);
}
void OnLostFocus()
{
	esLogMessage("[render]OnLostFocus fps %.3f", g_fps);
	g_fps = 0.0f;
	//g_device->AcqiureThreadOwnerShip();
}
void OnGainFocus()
{
	esLogMessage("[render]OnGainFocus");
	//g_device->ReleaseThreadOwnership();
}
bool inited = false;

int esMain(ESContext *esContext)
{
	esLogMessage("esMain ( ESContext *esContext )");

	DemoBase::Instnce()->OnCreateDevice(esContext);
	esRegisterShutdownFunc(esContext, Shutdown);
	esRegisterDrawFunc(esContext, Draw);
	esRegisterUpdateFunc(esContext, Update);

	return GL_TRUE;
}