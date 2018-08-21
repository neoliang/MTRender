#include "DemoBase.h"
#include "esUtil.h"
#include "ESDevice.hpp"
#include "ThreadESDevice.hpp"
#include "ThreadBufferESDevice.h"
#include <cmath>
#include <thread>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
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
#include "RingBuffer.h"
void DemoBase::Init()
{
	//≤‚ ‘ringbuffer
	auto fileContent = readFileData("monkey.babylon");
	RingBuffer* buffer = new RingBuffer(4096);
	std::thread s1 = std::thread([&]() {
		for (int i = 0; i < 10000; ++i)
		{
			auto v = buffer->ReadValueType<int>();
			buffer->ReadReleaseData();
			if (i >= 9999)
			{
				esLogMessage("ringbuffer v %d", v);
			}
		}
		char* buff = new char[fileContent.size()+1];
		
		buffer->ReadStreamingData(buff, fileContent.size());
		buff[fileContent.size()] = '\0';
		std::string newString(buff);
		if (newString == fileContent)
		{
			esLogMessage("readstreaming data equal");
		}
		else
		{
			esLogMessage("readstreaming data not equal");
		}
	});
	for (int i = 0; i < 10000; ++i)
	{
		buffer->WriteValueType(i);
		buffer->WriteSubmitData();
	}
	
	buffer->WriteStreamingData(fileContent.c_str(), fileContent.size());
	s1.join();
	delete buffer;

	_meshes = Mesh::LoadMeshFromFile("monkey.babylon");
	auto tM = _meshes[0];
	for (int i = 0; i < 40; ++i)
	{
		auto m = std::make_shared<Mesh>("cube", tM->vertices.size(), tM->indices.size());
		m->position = glm::vec3((rand() % 1000)/100.0f-5, (rand() % 1000)/100.0f-5, (rand() % 1000)/100.0f - 5);
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
		_device = new ThreadBufferESDevice(esContext, _returnResImmediately);
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
		((ThreadBufferESDevice*)_device)->Run();
#endif
	}
	_device->SetClearColor(0.0f, 0.0f, 0.6f, 0.0f);
	int width,dataLen,
		height;
	char *buffer = esLoadTGA(esContext->platformData, "basemap.tga", &width, &height,&dataLen);
	_texture = _device->CreateTexture2D(width, height, buffer,dataLen);
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
glm::mat4 g_mat;
void SimulateBusy()
{
	auto w0 = glm::translate(glm::mat4(1.0f), vec3(1.0,5.0,20));
	auto w1 = glm::eulerAngleXYZ(0.7f,0.8f,0.9f);
	for (int i = 0; i < 5000; ++i)
	{
		g_mat = w0*w1;
	}
}
void DemoBase::Update(float dt)
{
	//BeginProfile("SimulateBusy");
	SimulateBusy();
	//EndProfile();
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
	//esLogMessage("Update avgfps: %f currentFPs: %f delta: %f\n", avgFps, newFPs, delta);
}



static double s_createShaderAccTime = 0;
static unsigned int s_createShaderAccCount = 0;

void DemoBase::Render()
{
	_device->BeginRender();

	_device->Clear();
	_device->UseTexture2D(_texture);
	//BeginProfile("device.CreateGPUProgram");
	float createShaderTime = TimeSinceStartup();
	//auto program = _device->CreateGPUProgram(vShaderStr, fShaderStr);
	createShaderTime = TimeSinceStartup() - createShaderTime;
	++s_createShaderAccCount;
	s_createShaderAccTime += createShaderTime;
	float avgTime = s_createShaderAccTime / s_createShaderAccCount;
	//esLogMessage("device.CreateGPUProgram cost: %f avg: %f\n", createShaderTime, avgTime);
	_device->UseGPUProgram(_program);

	const glm::vec3 up(0, 1, 0);
	auto viewMat = glm::lookAt(_camera->position, _camera->target, up);
	auto mvpParam = _program->GetParam("MVP");
	glm::mat4 projMat = glm::perspective(glm::radians(45.0f), (float)_device->GetScreenWidth() / _device->GetScreenHeigt(), 0.1f, 20.0f);
	for (auto mesh : _meshes)
	{
		auto w0 = glm::translate(glm::mat4(1.0f), mesh->position);
		auto w1 = glm::eulerAngleXYZ(mesh->rotation.x, mesh->rotation.y, mesh->rotation.z);
		auto wordlMat = w0 * w1;
		auto mvp = projMat * viewMat* wordlMat;
		_device->SetGPUProgramParamAsMat4(mvpParam, mvp);
		_device->DrawVBO(mesh->vbo);
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