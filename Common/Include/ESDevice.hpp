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
#include "Camera.hpp"
#include "Mesh.hpp"


namespace RenderEngine {
	class ESDevice;	
	class ESDeviceImp;
	class GPUProgram
	{
		friend class ESDeviceImp;
	protected:
		virtual ~GPUProgram();
	};

	class Texture2D
	{
		friend class ESDeviceImp;
	protected:
		virtual ~Texture2D();
	};

	class ESDevice {
	public:
		virtual ~ESDevice() {};
		virtual bool CreateWindow1(const std::string& title, int width, int height, int flags) = 0;
		virtual void Clear() = 0;
		virtual void UseGPUProgram(GPUProgram* program) = 0;
		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader) = 0;
		virtual void DeletGPUProgram(GPUProgram* program) = 0;
		virtual Texture2D* CreateTexture2D(int width, int height, const void* data) = 0;
		virtual void DeleteTexture2D(Texture2D* texture) = 0;
		virtual void UseTexture2D(Texture2D* texture) = 0;		
		virtual void SetClearColor(float r, float g, float b, float alpha) = 0;
		virtual void DrawTriangle(std::vector<glm::vec3>& vertices)=0;
		virtual void SetViewPort(int x, int y, int width, int height) = 0;
		virtual void AcqiureThreadOwnerShip() =0;
		virtual void ReleaseThreadOwnership() =0;
		virtual void BeginRender() = 0;
		virtual void Present() = 0;
		virtual void Render(Camera::Ptr camer, const std::vector<Mesh::Ptr>& mesh) = 0;
		virtual void Cleanup() = 0;
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
		virtual void UseGPUProgram(GPUProgram* program);
		virtual GPUProgram* CreateGPUProgram(const std::string& vertexShader, const std::string& fragmentShader);
		virtual void DeletGPUProgram(GPUProgram* program);
		virtual Texture2D* CreateTexture2D(int width, int height, const void* data);
		virtual void DeleteTexture2D(Texture2D* texture);
		virtual void UseTexture2D(Texture2D* texture);
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
		virtual void Render(Camera::Ptr camer, const std::vector<Mesh::Ptr>& mesh);
	};
}
#endif /* ESDevice_hpp */
