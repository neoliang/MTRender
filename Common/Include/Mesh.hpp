
#ifndef Mesh_hpp
#define Mesh_hpp
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include <memory>
namespace RenderEngine {
	class Mesh
	{
	public:
		typedef std::shared_ptr<Mesh> Ptr;

	public:
		static std::vector <Ptr> LoadMeshFromFile(const std::string& file);
	public:
		std::string name;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<unsigned short> indices;
		glm::vec3 position;
		glm::vec3 rotation;
		Mesh(const std::string& name_, int vericesCount, int indicesCount = 0)
			:name(name_), position(0, 0, 0), rotation(0, 0, 0)
		{
			vertices.resize(vericesCount);
			uvs.resize(vericesCount);
			indices.resize(indicesCount);
		}
		~Mesh()
		{
		}
	};
}
#endif