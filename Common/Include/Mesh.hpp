
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
		std::vector<Vertex> vertices;
		std::vector<unsigned short> indices;
		typedef std::shared_ptr<VBOData> Ptr;


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
			vboData = std::make_shared<VBOData>();
			vboData->vertices.resize(vericesCount);
			vboData->indices.resize(indicesCount);
		}
		~Mesh()
		{
		}
	};
}
#endif