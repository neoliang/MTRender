
#ifndef Mesh_hpp
#define Mesh_hpp
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include <memory>
#include <GLES3/gl3.h>
namespace RenderEngine {

	class VBO
	{
	protected:
		virtual ~VBO() {}
	public:
		virtual VBO* GetRealVBO() = 0;

	};

	struct VBOData
	{
		struct Vertex
		{
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv;
		};
		VBOData(unsigned int verticesCount_, unsigned int indicesCount_)
			:verticesCount(verticesCount_), indicesCount(indicesCount_) 
		{
			_buffer = new char[verticesCount * sizeof(Vertex) + indicesCount * sizeof(unsigned short)];
			vertices = (Vertex*)_buffer;
			indices = (unsigned short*)(_buffer + verticesCount * sizeof(Vertex));
		}
		~VBOData()
		{
			delete[] _buffer;
		}
		Vertex* vertices;
		unsigned short* indices;
		unsigned int verticesCount;
		unsigned int indicesCount;
		typedef std::shared_ptr<VBOData> Ptr;

	private:
		char* _buffer;
	};

	class Mesh
	{
	public:
		typedef std::shared_ptr<Mesh> Ptr;

	public:
		static std::vector <Ptr> LoadMeshFromFile(const std::string& file);
	public:
		std::string name;
		std::shared_ptr<VBOData> vboData;

		glm::vec3 position;
		glm::vec3 rotation;
		VBO* vbo;
		Mesh(const std::string& name_, int vericesCount, int indicesCount = 0)
			:name(name_), position(0, 0, 0), rotation(0, 0, 0)
		{
			vboData = std::make_shared<VBOData>(vericesCount, indicesCount);
		}
		~Mesh()
		{
		}
	};
}
#endif