//
//  ESDevice.hpp
//  Hello_Triangle
//
//  Created by tencent on 2018/3/22.
//  Copyright © 2018年 Daniel Ginsburg. All rights reserved.
//

#ifndef ESDevice_hpp
#define ESDevice_hpp
#include "esUtil.h"
#include <string>
#include "glm/glm.hpp"
#include "Mesh.hpp"

namespace RenderEngine {
	class ESDevice;	
	class ESDeviceImp;

	class GPUProgramParam
	{
	protected:
		virtual ~GPUProgramParam() {}
	public:
		virtual GPUProgramParam* GetRealParam() = 0;
	};

	class GPUProgram
	{
		friend class ESDeviceImp;
	protected:
		virtual ~GPUProgram() {};
	public:
		virtual GPUProgram* GetRealGUPProgram() = 0;
		virtual GPUProgramParam* GetParam(const std::string& name) = 0;
	};

	class Texture2D
	{
		friend class ESDeviceImp;
	protected:
		virtual ~Texture2D() {};
	public:
		virtual Texture2D* GetRealTexture2D() = 0;
	};

	struct TextureData
	{
		char* pixels;
		unsigned int length;
		unsigned int width;
		unsigned int height;
		//TO DO format desc

		TextureData()
			:pixels(nullptr),length(0),width(0),height(0)
		{

		}
		TextureData(char* pixels_, unsigned int width_, unsigned int height_, unsigned int lenght_)
			:pixels(pixels_), width(width_), height(height_), length(lenght_) {}
		~TextureData()
		{
			delete[] pixels;
		}
		typedef std::shared_ptr<TextureData> Ptr;
	private:
		TextureData(TextureData&);
		const TextureData& operator=(const TextureData&);
	};

	class ESDevice {
	public:
		virtual ~ESDevice() {};
		virtual bool CreateWindow1(const std::string& title, int width, int height, int flags) = 0;
		virtual void Clear() = 0;

		virtual Texture2D* CreateTexture2D(const TextureData::Ptr& data) = 0;
		virtual void DeleteTexture2D(Texture2D* texture) = 0;
		virtual void UseTexture2D(Texture2D* texture,unsigned int index) = 0;
		virtual void SetClearColor(float r, float g, float b, float alpha) = 0;
		virtual void DrawTriangle(std::vector<glm::vec3>& vertices)=0;
		virtual void SetViewPort(int x, int y, int width, int height) = 0;
		virtual void AcqiureThreadOwnerShip() =0;
		virtual void ReleaseThreadOwnership() =0;
		virtual void BeginRender() = 0;
		virtual void Present() = 0;
		virtual VBO* CreateVBO()=0;
		virtual void UpdateVBO(VBO* vbo,const VBOData::Ptr& vboData)=0;
		virtual void DeleteVBO(VBO* vbo) = 0;
		virtual void DrawVBO(VBO* vbo) = 0;
		virtual void Cleanup() = 0;

		virtual void UseGPUProgram(GPUProgram* program) = 0;
		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader) = 0;
		virtual void DeletGPUProgram(GPUProgram* program) = 0;
		virtual GPUProgramParam* GetGPUProgramParam(GPUProgram* program, const std::string& name) = 0;
		virtual void SetGPUProgramParamAsInt(GPUProgramParam* param, int value) = 0;

		virtual void SetGPUProgramParamAsFloat(GPUProgramParam* param, float value) = 0;

		virtual void SetGPUProgramParamAsMat4(GPUProgramParam* param, const glm::mat4& mat) = 0;

		virtual void SetGPUProgramParamAsIntArray(GPUProgramParam* param, const std::vector<int>& values) = 0;

		virtual void SetGPUProgramParamAsFloatArray(GPUProgramParam* param, const std::vector<float>& values) = 0;

		virtual void SetGPUProgramParamAsMat4Array(GPUProgramParam* param,  const std::vector<glm::mat4>& values) = 0;
		virtual int GetScreenWidth() = 0;
		virtual int GetScreenHeigt() = 0;
	};

	class ESDeviceImp : public ESDevice
	{
	private:
		ESContext * _esContext;
	public:
		ESDeviceImp(ESContext* context) :_esContext(context) {
			esLogMessage("ESDeviceImp");
		};
		~ESDeviceImp() {
			esLogMessage("~ESDeviceImp");
		}
		virtual void Cleanup() {}
		virtual bool CreateWindow1(const std::string& title, int width, int height, int flags);
		virtual void Clear();

		virtual Texture2D* CreateTexture2D(const TextureData::Ptr& data);
		virtual void DeleteTexture2D(Texture2D* texture);
		virtual void UseTexture2D(Texture2D* texture,unsigned int index);
		virtual void SetClearColor(float r, float g, float b, float alpha);
		virtual void DrawTriangle(std::vector<glm::vec3>& vertices);
		virtual void SetViewPort(int x, int y, int width, int height);
		virtual void AcqiureThreadOwnerShip();
		virtual void ReleaseThreadOwnership();
		virtual void BeginRender() {};
		virtual void Present();
		virtual void Draw2DPoint(const glm::vec2& pos);
		virtual void DrawLine(const std::vector<glm::vec3>& line);
		virtual glm::vec3 Project(const glm::vec3& coord, const glm::mat4& transMat);
		virtual VBO* CreateVBO();
		virtual void UpdateVBO(VBO* vbo, const VBOData::Ptr& vboData);
		virtual void DeleteVBO(VBO* vbo);
		virtual void DrawVBO(VBO* vbo);
		virtual int GetScreenWidth();
		virtual int GetScreenHeigt();
		//gpu program
		virtual void UseGPUProgram(GPUProgram* program);

		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader);

		virtual void DeletGPUProgram(GPUProgram* program);

		virtual GPUProgramParam* GetGPUProgramParam(GPUProgram* program, const std::string& name);

		virtual void SetGPUProgramParamAsInt(GPUProgramParam* param, int value);

		virtual void SetGPUProgramParamAsFloat(GPUProgramParam* param, float value);

		virtual void SetGPUProgramParamAsMat4(GPUProgramParam* param, const glm::mat4& mat);

		virtual void SetGPUProgramParamAsIntArray(GPUProgramParam* param, const std::vector<int>& values);

		virtual void SetGPUProgramParamAsFloatArray(GPUProgramParam* param,  const std::vector<float>& values);

		virtual void SetGPUProgramParamAsMat4Array(GPUProgramParam* param, const std::vector<glm::mat4>& values);
	};
}
#endif /* ESDevice_hpp */
