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
"layout(location = 0) in vec3 vPosition;  \n"
"layout(location = 1) in vec2 a_texCoord; \n"
"out vec2 v_texCoord;                     \n"
"	uniform mat4 MVP;                     \n"
"void main()                              \n"
"{                                        \n"
"   gl_Position = MVP*vec4(vPosition,1);		  \n"
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
	meshes = Mesh::LoadMeshFromFile("monkey.babylon");
	auto m = meshes[0];
	for (int i = 0; i < m->vertices.size(); ++i)
	{
		printf("%1f,%1f,%1f, %1f, %1f\n", m->vertices[i].x, m->vertices[i].y, m->vertices[i].z, m->uvs[i].x, m->uvs[i].y);
	}
	camera = Camera::Ptr(new Camera);
	camera->position = vec3(0, 0, 5.0f);
	camera->target = vec3(0, 0, 0);
}

void DemoBase::OnCreateDevice(ESContext *esContext)
{
#ifdef __APPLE__
	g_device = new ESDeviceImp(esContext);
#else
	if (_multiThreaded)
	{
		g_device = new ThreadESDevice(esContext, _returnResImmediately);
	}
	else
	{
		g_device = new ESDeviceImp(esContext);
	}
#endif
	g_device->CreateWindow1("Hello Triangle", 480, 320, ES_WINDOW_RGB | ES_WINDOW_DEPTH | ES_WINDOW_ALPHA);
	if (!_multiThreaded)
	{
		g_device->AcqiureThreadOwnerShip();
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
	}
	else
	{
#ifndef __APPLE__
		((ThreadESDevice*)g_device)->Run();
#endif
	}
	g_device->SetClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	int width,
		height;

	char *buffer = esLoadTGA(esContext->platformData, "basemap.tga", &width, &height);
	g_texture = g_device->CreateTexture2D(width, height, buffer);
	g_program = g_device->CreateGPUProgram(vShaderStr, fShaderStr);
	for (auto mesh : meshes)
	{
		mesh->vbo = g_device->CreateVBO(mesh->vertices, mesh->uvs, mesh->indices);
	}
}

void DemoBase::OnDestroyDevice()
{
	g_device->DeletGPUProgram(g_program);
	g_device->DeleteTexture2D(g_texture);
	for (auto mesh : meshes)
	{
		g_device->DeleteVBO(mesh->vbo);
	}
	g_device->Cleanup();
	delete g_device;
	g_device = nullptr;
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
	for (auto mesh : meshes)
	{
		mesh->rotation = vec3(mesh->rotation.x + dt * 0.5f, mesh->rotation.y + dt * 0.2f, mesh->rotation.z);
	}
	esLogMessage("Update avgfps: %f currentFPs: %f delta: %f\n", avgFps, newFPs, delta);
	ESSleep(0.02f);
}

void DemoBase::Render()
{
	//BeginProfile("g_device->BeginRender()");
	g_device->BeginRender();
	//EndProfile();
	//g_device->SetViewPort(0, 0, esContext->width, esContext->height);

	g_device->Clear();
	g_device->UseTexture2D(g_texture);
	auto program = g_device->CreateGPUProgram(vShaderStr, fShaderStr);
	g_device->UseGPUProgram(program);
	g_device->DeletGPUProgram(program);
	for (int i = 0; i < 40; ++i)
	{
		g_device->Render(camera, meshes);
	}
	g_device->Present();
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