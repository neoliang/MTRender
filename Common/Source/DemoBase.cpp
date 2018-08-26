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

const std::string vStr =
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

const std::string fStr =
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

DemoBase::DemoBase(bool returnResImmediately,DeviceCreateType type)
:_returnResImmediately(returnResImmediately)
,_deviceCreateType(type)
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

VBOData::Ptr _vboData = std::make_shared<VBOData>();
glm::mat4 _mvp;
VBO* _vbo;
void DemoBase::Init()
{
	_meshes = Mesh::LoadMeshFromFile("monkey.babylon");
	_vboData->vertices = _meshes[0]->vboData->vertices;
	_vboData->indices = _meshes[0]->vboData->indices;
	for (int i = 0; i < 40; ++i)
	{
		_vboData->vertices.insert(_vboData->vertices.end(), _meshes[0]->vboData->vertices.begin(),_meshes[0]->vboData->vertices.end());
		_vboData->indices.insert(_vboData->indices.end(), _meshes[0]->vboData->indices.begin(), _meshes[0]->vboData->indices.end());
	}
	_camera = Camera::Ptr(new Camera);
	_camera->position = vec3(0, 0, 12.0f);
	_camera->target = vec3(0, 0, 0);
}
TextureData::Ptr g_textureData;
void DemoBase::OnCreateDevice(ESContext *esContext)
{
#ifdef __APPLE__
	_device = new ESDeviceImp(esContext);
#else
	switch (_deviceCreateType)
	{
	case DemoBase::kThreadBuffer:
		_device = new ThreadBufferESDevice(esContext, _returnResImmediately);
		break;
	case DemoBase::kThreadQueue:
		_device = new ThreadESDevice(esContext, _returnResImmediately);
		break;
	default:
		_device = new ESDeviceImp(esContext);
		break;
	}
#endif
	ThreadESDeviceBase* threadDevice = dynamic_cast<ThreadESDeviceBase*>(_device);

	_device->CreateWindow1("Hello Triangle", 480, 320, ES_WINDOW_RGB | ES_WINDOW_DEPTH | ES_WINDOW_ALPHA);
	if (threadDevice == nullptr)
	{
		_device->AcqiureThreadOwnerShip();
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
	}

#ifndef __APPLE__
	if (threadDevice != nullptr)
	{
		threadDevice->Run();
	}
#endif
	_device->SetClearColor(0.0f, 0.0f, 0.6f, 0.0f);
	int width,dataLen,
		height;
	char *buffer = esLoadTGA(esContext->platformData, "splash04.tga", &width, &height,&dataLen);
	g_textureData  = std::make_shared<TextureData>(buffer, width, height, dataLen);
	_texture = _device->CreateTexture2D(g_textureData);
	_program = _device->CreateGPUProgram(vStr, fStr);
	for (auto mesh : _meshes)
	{
		mesh->vbo = _device->CreateVBO();
		_device->UpdateVBO(mesh->vbo, _vboData);
	}

	const glm::vec3 up(0, 1, 0);
	auto viewMat = glm::lookAt(_camera->position, _camera->target, up);
	_device->UseGPUProgram(_program);
	auto mvpParam = _program->GetParam("MVP");
	
	glm::mat4 projMat = glm::perspective(glm::radians(45.0f), (float)_device->GetScreenWidth() / _device->GetScreenHeigt(), 0.1f, 20.0f);
	auto mesh = _meshes[0];

	auto w0 = glm::translate(glm::mat4(1.0f), mesh->position);
	auto w1 = glm::eulerAngleXYZ(mesh->rotation.x, mesh->rotation.y, mesh->rotation.z);
	auto wordlMat = w0 * w1;
	_mvp = projMat * viewMat* wordlMat;
	_device->SetGPUProgramParamAsMat4(mvpParam, _mvp);
	_device->UseTexture2D(_texture,1);
	auto textParam = _program->GetParam("baseTex");
	_device->SetGPUProgramParamAsInt(textParam,1);
	_vbo = mesh->vbo;
}

void DemoBase::OnDestroyDevice()
{
	_device->DeletGPUProgram(_program);
	if (_texture != nullptr)
	{
		_device->DeleteTexture2D(_texture);
	}	
	for (auto mesh : _meshes)
	{
		_device->DeleteVBO(mesh->vbo);
	}
	_device->Cleanup();
	delete _device;
	_device = nullptr;
}

static double g_accTime = 0;
static unsigned int g_accCount = 0;
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
float _rotaion = 0 ;
void DemoBase::Update(float dt)
{
	_rotaion += dt * M_PI / 2.0f;
	//BeginProfile("SimulateBusy");
	SimulateBusy();
	//EndProfile();
	++g_accCount;
	g_accTime += dt;
	float avgFps = 1.0f / (g_accTime / g_accCount);
	float newFPs = 1.0f / dt;
	float delta = newFPs - g_fps;
	g_fps = newFPs;
	for (int i = 0; i < _meshes[0]->vboData->vertices.size(); ++i)
	{
		_meshes[0]->vboData->vertices[i].pos.x = sinf(_rotaion) * _vboData->vertices[i].pos.x;
		_meshes[0]->vboData->vertices[i].pos.y = cosf(_rotaion) * _vboData->vertices[i].pos.y;
	}
	for (auto mesh : _meshes)
	{
		//mesh->rotation = vec3(mesh->rotation.x + dt * 0.5f, mesh->rotation.y + dt * 0.2f, mesh->rotation.z);
	}
	if (g_accCount % 200 == 0)
	{
		esLogMessage("Update avgfps: %f currentFPs: %f delta: %f %u\n", avgFps, newFPs, delta,g_accCount);
	}
	if (g_accCount >= 2000)
	{
		g_accCount = 0;
		g_accTime = 0;
	}
}




void DemoBase::Render()
{
	_device->Clear();
	_device->UpdateVBO(_vbo,_vboData);
	_device->DrawVBO(_vbo);
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