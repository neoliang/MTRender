//
//  ESDevice.cpp
//  Hello_Triangle
//
//  Created by tencent on 2018/3/22.
//  Copyright © 2018年 Daniel Ginsburg. All rights reserved.
//

#include "ESDevice.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include <algorithm>
namespace RenderEngine {

	class GPUProgramImp : public GPUProgram
	{
	public:
		GLuint ProgramID;
		GLuint textLoc;
		GPUProgramImp(GLuint id)
			:ProgramID(id) {
			textLoc = glGetUniformLocation(ProgramID, "baseTex");
			//esLogMessage("textLoc %d",(int)textLoc);
		}
		GPUProgram* GetRealGUPProgram()
		{
			return this;
		}
	};
	GPUProgram::~GPUProgram()
	{
		//esLogMessage("[render]~GPUProgram()");
	}


	class Texture2DImp : public Texture2D
	{
	public:
		GLuint textureID;
		Texture2DImp(GLuint id)
			:textureID(id) {}

		Texture2D* GetRealTexture2D()
		{
			return this;
		}
	};

	Texture2D::~Texture2D()
	{
		esLogMessage("[render]~Texture2D()");
	}


	bool ESDeviceImp::CreateWindow1(const std::string& title, int width, int height, int flags)
	{
		return esCreateWindow(_esContext, title.c_str(), width, height, flags);
	}
	void ESDeviceImp::Clear()
	{
		glClearDepthf(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void ESDeviceImp::SetViewPort(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
	}

	GPUProgram* ESDeviceImp::CreateGPUProgram(const std::string& vertexShaderStr, const std::string& fragmentShaderStr)
	{
		auto vertexShader = esLoadShader(GL_VERTEX_SHADER, vertexShaderStr.c_str());
		auto fragmentShader = esLoadShader(GL_FRAGMENT_SHADER, fragmentShaderStr.c_str());
		auto programObject = glCreateProgram();

		if (programObject != 0)
		{


			glAttachShader(programObject, vertexShader);
			glAttachShader(programObject, fragmentShader);

			// Link the program
			glLinkProgram(programObject);

			// Check the link status
			GLint linked;
			glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

			if (!linked)
			{
				GLint infoLen = 0;

				glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

				if (infoLen > 1)
				{
					char *infoLog = (char*)malloc(sizeof(char) * infoLen);

					glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
					esLogMessage("Error linking program:\n%s\n", infoLog);

					free(infoLog);
				}

				glDeleteProgram(programObject);
				programObject = 0;
			}
		}
		return new GPUProgramImp(programObject);
	}
	GLuint _programId;
	void ESDeviceImp::UseGPUProgram(GPUProgram* program)
	{
		_programId = static_cast<GPUProgramImp*>(program)->ProgramID;

		glUseProgram(_programId);
		glUniform1i(static_cast<GPUProgramImp*>(program)->textLoc, 0);
	}
	void ESDeviceImp::DeletGPUProgram(GPUProgram* program)
	{
		glDeleteProgram(static_cast<GPUProgramImp*>(program)->ProgramID);
		delete program;
	}
	Texture2D* ESDeviceImp::CreateTexture2D(int width, int height, const void* data)
	{
		GLuint textureID = 0;
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		esLogMessage("textureID %d", (int)textureID);
		return new Texture2DImp(textureID);
	}
	void ESDeviceImp::DeleteTexture2D(Texture2D* texture)
	{
		Texture2DImp* realTex = static_cast<Texture2DImp*>(texture);
		glDeleteTextures(1, &realTex->textureID);
		delete texture;
	}
	void ESDeviceImp::UseTexture2D(Texture2D* texture)
	{
		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, static_cast<Texture2DImp*>(texture)->textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	void ESDeviceImp::SetClearColor(float r, float g, float b, float alpha)
	{
		glClearColor(r, g, b, alpha);
	}
	void ESDeviceImp::DrawTriangle(std::vector<glm::vec3>& vertices)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &vertices[0][0]);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_LINES, 0, 2);
	}
	void ESDeviceImp::Present()
	{
#ifndef __APPLE__
		eglSwapBuffers(_esContext->eglDisplay, _esContext->eglSurface);
#endif
	}
	void RenderEngine::ESDeviceImp::Draw2DPoint(const glm::vec2 & pos)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &pos[0]);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_POINTS, 0, 1);
	}
	void RenderEngine::ESDeviceImp::DrawLine(const std::vector<glm::vec3>& line)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &line[0][0]);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_LINES, 0, 2);
	}
	glm::vec3 RenderEngine::ESDeviceImp::Project(const glm::vec3 & coord, const glm::mat4 & transMat)
	{
		auto pos = transMat * glm::vec4(coord, 1.0f);
		return glm::vec3(pos.x, pos.y, pos.z) / pos.w;
	}
	VBO* ESDeviceImp::CreateVBO(std::vector<glm::vec3> vertices, std::vector<glm::vec2> uvs, std::vector<unsigned short> indices)
	{
		auto vbo = new VBOImp();
		glGenVertexArrays(1, &vbo->vertexArrayID);
		glBindVertexArray(vbo->vertexArrayID);

		glGenBuffers(1, &vbo->vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vbo->vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &vbo->uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vbo->uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);


		glGenBuffers(1, &vbo->elementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
		vbo->elementSize = indices.size();
		return vbo;
	}
	void ESDeviceImp::DeleteVBO(VBO* vbo)
	{
		VBOImp* vboImp = static_cast<VBOImp*>(vbo);
		glDeleteBuffers(1, &vboImp->vertexbuffer);
		glDeleteBuffers(1, &vboImp->elementbuffer);
		glDeleteBuffers(1, &vboImp->uvbuffer);
		glDeleteVertexArrays(1, &vboImp->vertexArrayID);
		delete vboImp;
	}
	void ESDeviceImp::DrawVBO(VBO* vbo)
	{
		VBOImp* vboImp = static_cast<VBOImp*>(vbo);
		glBindVertexArray(vboImp->vertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, vboImp->vertexbuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, vboImp->uvbuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboImp->elementbuffer);
		glDrawElements(GL_TRIANGLES, vboImp->elementSize, GL_UNSIGNED_SHORT, (void*)0);
	}
	void ESDeviceImp::Render(Camera::Ptr camer, const std::vector<Mesh::Ptr>& meshes)
	{
		//DrawLine({ glm::vec3(0,0,0),glm::vec3(1,1,0) });
		const glm::vec3 up(0, 1, 0);
		auto viewMat = glm::lookAt(camer->position, camer->target, up);
		GLuint MatrixID = glGetUniformLocation(_programId, "MVP");
		glm::mat4 projMat = glm::perspective(glm::radians(45.0f), (float)_esContext->width / _esContext->height, 0.1f, 100.0f);
		for (auto mesh : meshes)
		{
			auto w0 = glm::translate(glm::mat4(1.0f), mesh->position);
			auto w1 = glm::eulerAngleXYZ(mesh->rotation.x, mesh->rotation.y, mesh->rotation.z);
			auto wordlMat = w0 * w1;
			auto mvp = projMat * viewMat* wordlMat;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
			this->DrawVBO(mesh->vbo->GetRealVBO());
			//static const GLfloat g_vertex_buffer_data[] = {
			//	-1.0f,-1.0f,-1.0f,
			//	-1.0f,-1.0f, 1.0f,
			//	-1.0f, 1.0f, 1.0f,
			//	1.0f, 1.0f,-1.0f,
			//	-1.0f,-1.0f,-1.0f,
			//	-1.0f, 1.0f,-1.0f,
			//	1.0f,-1.0f, 1.0f,
			//	-1.0f,-1.0f,-1.0f,
			//	1.0f,-1.0f,-1.0f,
			//	1.0f, 1.0f,-1.0f,
			//	1.0f,-1.0f,-1.0f,
			//	-1.0f,-1.0f,-1.0f,
			//	-1.0f,-1.0f,-1.0f,
			//	-1.0f, 1.0f, 1.0f,
			//	-1.0f, 1.0f,-1.0f,
			//	1.0f,-1.0f, 1.0f,
			//	-1.0f,-1.0f, 1.0f,
			//	-1.0f,-1.0f,-1.0f,
			//	-1.0f, 1.0f, 1.0f,
			//	-1.0f,-1.0f, 1.0f,
			//	1.0f,-1.0f, 1.0f,
			//	1.0f, 1.0f, 1.0f,
			//	1.0f,-1.0f,-1.0f,
			//	1.0f, 1.0f,-1.0f,
			//	1.0f,-1.0f,-1.0f,
			//	1.0f, 1.0f, 1.0f,
			//	1.0f,-1.0f, 1.0f,
			//	1.0f, 1.0f, 1.0f,
			//	1.0f, 1.0f,-1.0f,
			//	-1.0f, 1.0f,-1.0f,
			//	1.0f, 1.0f, 1.0f,
			//	-1.0f, 1.0f,-1.0f,
			//	-1.0f, 1.0f, 1.0f,
			//	1.0f, 1.0f, 1.0f,
			//	-1.0f, 1.0f, 1.0f,
			//	1.0f,-1.0f, 1.0f
			//};

			//// Two UV coordinatesfor each vertex. They were created with Blender.
			//static const GLfloat g_uv_buffer_data[] = {
			//	0.000059f, 1.0f - 0.000004f,
			//	0.000103f, 1.0f - 0.336048f,
			//	0.335973f, 1.0f - 0.335903f,
			//	1.000023f, 1.0f - 0.000013f,
			//	0.667979f, 1.0f - 0.335851f,
			//	0.999958f, 1.0f - 0.336064f,
			//	0.667979f, 1.0f - 0.335851f,
			//	0.336024f, 1.0f - 0.671877f,
			//	0.667969f, 1.0f - 0.671889f,
			//	1.000023f, 1.0f - 0.000013f,
			//	0.668104f, 1.0f - 0.000013f,
			//	0.667979f, 1.0f - 0.335851f,
			//	0.000059f, 1.0f - 0.000004f,
			//	0.335973f, 1.0f - 0.335903f,
			//	0.336098f, 1.0f - 0.000071f,
			//	0.667979f, 1.0f - 0.335851f,
			//	0.335973f, 1.0f - 0.335903f,
			//	0.336024f, 1.0f - 0.671877f,
			//	1.000004f, 1.0f - 0.671847f,
			//	0.999958f, 1.0f - 0.336064f,
			//	0.667979f, 1.0f - 0.335851f,
			//	0.668104f, 1.0f - 0.000013f,
			//	0.335973f, 1.0f - 0.335903f,
			//	0.667979f, 1.0f - 0.335851f,
			//	0.335973f, 1.0f - 0.335903f,
			//	0.668104f, 1.0f - 0.000013f,
			//	0.336098f, 1.0f - 0.000071f,
			//	0.000103f, 1.0f - 0.336048f,
			//	0.000004f, 1.0f - 0.671870f,
			//	0.336024f, 1.0f - 0.671877f,
			//	0.000103f, 1.0f - 0.336048f,
			//	0.336024f, 1.0f - 0.671877f,
			//	0.335973f, 1.0f - 0.335903f,
			//	0.667969f, 1.0f - 0.671889f,
			//	1.000004f, 1.0f - 0.671847f,
			//	0.667979f, 1.0f - 0.335851f
			//};

			//GLuint vertexbuffer;
			//glGenBuffers(1, &vertexbuffer);
			//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

			//GLuint uvbuffer;
			//glGenBuffers(1, &uvbuffer);
			//glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

			//glEnableVertexAttribArray(0);
			//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			//glVertexAttribPointer(
			//	0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			//	3,                  // size
			//	GL_FLOAT,           // type
			//	GL_FALSE,           // normalized?
			//	0,                  // stride
			//	(void*)0            // array buffer offset
			//);

			//// 2nd attribute buffer : UVs
			//glEnableVertexAttribArray(1);
			//glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			//glVertexAttribPointer(
			//	1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			//	2,                                // size : U+V => 2
			//	GL_FLOAT,                         // type
			//	GL_FALSE,                         // normalized?
			//	0,                                // stride
			//	(void*)0                          // array buffer offset
			//);

			//// Draw the triangle !
			//glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

			//glDisableVertexAttribArray(0);
			//glDisableVertexAttribArray(1);
			//glDeleteBuffers(1, &vertexbuffer);
			//glDeleteBuffers(1, &uvbuffer);

		}
	}
	void ESDeviceImp::AcqiureThreadOwnerShip()
	{
#ifndef __APPLE__
		eglMakeCurrent(_esContext->eglDisplay, _esContext->eglSurface,
			_esContext->eglSurface, _esContext->eglContext);
#endif
	}
	void ESDeviceImp::ReleaseThreadOwnership()
	{
#ifndef __APPLE__
		eglMakeCurrent(_esContext->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#endif
	}

}
