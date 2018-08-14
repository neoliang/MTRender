// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
// Hello_Triangle.c
//
//    This is a simple example that draws a single triangle with
//    a minimal vertex/fragment shader.  The purpose of this
//    example is to demonstrate the basic concepts of
//    OpenGL ES 3.0 rendering.
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

ESDevice* g_device;
GPUProgram* g_program;
Texture2D* g_texture;
///
// Initialize the shader and program object
//
const std::string vShaderStr =
"#version 300 es                          \n"
"layout(location = 0) in vec4 vPosition;  \n"
"layout(location = 1) in vec2 a_texCoord; \n"
"out vec2 v_texCoord;                     \n"
"	uniform mat4 MVP;                     \n"
"void main()                              \n"
"{                                        \n"
"   gl_Position = MVP*vPosition;		  \n"
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
int Init (ESContext* esContext )
{
    g_device->SetClearColor ( 0.0f, 0.0f, 1.0f, 0.0f );
	int width,
		height;

	char *buffer = esLoadTGA(esContext->platformData, "Suzanne.tga", &width, &height);
	g_texture = g_device->CreateTexture2D(width, height, buffer);
	g_program = g_device->CreateGPUProgram(vShaderStr, fShaderStr);
    return 1;
}

///
// Draw a triangle using the shader pair created in Init()
std::vector<Mesh::Ptr> meshes; //= Mesh::Ptr(new Mesh("Cube", 8, 36));
Camera::Ptr camera; //= Camera::Ptr(new Camera);

glm::vec3 ovVertices[] = {
	{ 0.0f,  0.5f, 0.0f },
{ -0.5f, -0.5f, 0.0f, },
{ 0.5f, -0.5f, 0.0f }
};

std::vector<glm::vec3> vVertices = { {0.0f,  0.5f, 0.0f},
{-0.5f, -0.5f, 0.0f},
{0.5f, -0.5f, 0.0f}
};
float angle = 0;
void Draw ( ESContext *esContext )
{

	g_device->SetViewPort(0, 0, esContext->width, esContext->height);
    
    // Clear the color buffer
	g_device->Clear();
	g_device->UseTexture2D(g_texture);
	g_device->UseGPUProgram(g_program);
    //g_device->DrawTriangle(vVertices);
    //g_device->DrawTriangle(vVertices1, sizeof(vVertices1)/sizeof(GLfloat));
	g_device->Render(camera, meshes);
	g_device->Present();
}

void Shutdown ( ESContext *esContext )
{
	esLogMessage("Shutdown");
    g_device->DeletGPUProgram(g_program);
	g_device->DeleteTexture2D(g_texture);
	g_device->Cleanup();
    delete g_device;
}

void Update(ESContext* esContext,float dt)
{
	for (auto mesh : meshes)
	{
		mesh->rotation = vec3(mesh->rotation.x + 0.01f, mesh->rotation.y + 0.01f, mesh->rotation.z);
	}
}
void OnLostFocus()
{
	esLogMessage("[render]OnLostFocus");
	//g_device->AcqiureThreadOwnerShip();
}
void OnGainFocus()
{
	esLogMessage("[render]OnGainFocus");
	//g_device->ReleaseThreadOwnership();
}
bool inited = false;
void InitApp()
{
	if (inited)
	{
		return;
	}
	inited = true;
	//mesh = Mesh::Ptr(new Mesh("Cube", 8, 36));
	meshes = Mesh::LoadMeshFromFile("monkey.babylon");

	camera = Camera::Ptr(new Camera);
	//    mesh->vertices[0] = vec3(-1, 1, 1);
	//    mesh->vertices[1] = vec3(1, 1, 1);
	//    mesh->vertices[2] = vec3(1, -1, 1);
	//    mesh->vertices[3] = vec3(-1, -1, 1);
	//    mesh->vertices[4] = vec3(-1, 1, -1);
	//    mesh->vertices[5] = vec3(1, 1, -1);
	//    mesh->vertices[6] = vec3(1, -1, -1);
	//    mesh->vertices[7] = vec3(-1, -1, -1   
	camera->position = vec3(0, 0, 5.0f);
	camera->target = vec3(0, 0, 0);
	//    mesh->indices = {
	// 	   0,1,2,0,2,3,0,4,7,0,7,3,4,5,7,4,6,7,
	// 	   0,4,5,0,5,1,5,6,2,5,2,1,6,2,3,6,3,7
	//    };
}
int esMain ( ESContext *esContext )
{
	esLogMessage("esMain ( ESContext *esContext )");
	
#ifdef __APPLE__
	g_device = new ESDeviceImp(esContext);
#else
	g_device = new ThreadESDevice(esContext);
   
#endif
	g_device->CreateWindow1 ( "Hello Triangle", 480, 320, ES_WINDOW_RGB );
   //g_device->AcqiureThreadOwnerShip();
#ifndef __APPLE__
   ((ThreadESDevice*)g_device)->Run();
#endif
   InitApp();
   if ( !Init (esContext  ) )
   {
       return GL_FALSE;
   }

   esRegisterShutdownFunc ( esContext, Shutdown );
   esRegisterDrawFunc ( esContext, Draw );
   esRegisterUpdateFunc(esContext, Update);
    
    return GL_TRUE;
}

