#include "Mesh.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "esUtil.h"

using namespace RenderEngine;
using namespace  rapidjson;
std::vector<Mesh::Ptr> Mesh::LoadMeshFromFile(const std::string & file)
{
	auto meshes = std::vector<Mesh::Ptr>();
	auto data = readFileData(file);
	Document d;
	d.Parse(data.c_str());

	Value& meshValues = d["meshes"];
	for (auto iter = meshValues.Begin(); iter != meshValues.End(); ++iter)
	{
		auto verticesArray = (*iter)["vertices"].GetArray();
		// Faces
		auto indicesArray = (*iter)["indices"].GetArray();
		auto uvCount = (*iter)["uvCount"].GetInt();
		auto verticesStep = 1;

		// Depending of the number of texture's coordinates per vertex
		// we're jumping in the vertices array  by 6, 8 & 10 windows frame
		switch ((int)uvCount)
		{
		case 0:
			verticesStep = 6;
			break;
		case 1:
			verticesStep = 8;
			break;
		case 2:
			verticesStep = 10;
			break;
		}
		// the number of interesting vertices information for us
		auto verticesCount = verticesArray.Size() / verticesStep;
		// number of faces is logically the size of the array divided by 3 (A, B, C)
		auto mesh = std::make_shared<Mesh>( (*iter)["name"].GetString(), verticesCount, indicesArray.Size());

		// Filling the Vertices array of our mesh first
		for (unsigned index = 0; index < verticesCount; index++)
		{
			float x = verticesArray[index * verticesStep].GetFloat();
			float y = verticesArray[index * verticesStep + 1].GetFloat();
			float z = verticesArray[index * verticesStep + 2].GetFloat();
			mesh->vertices[index] = glm::vec3(x, y, z);
			if (uvCount > 0)
			{
				float u = verticesArray[index * verticesStep + 6].GetFloat();
				float v = verticesArray[index * verticesStep + 7].GetFloat();
				mesh->uvs[index] = glm::vec2(u, v);
			}
		}

		// Then filling the Faces array
		for (unsigned index = 0; index < indicesArray.Size(); index++)
		{
			mesh->indices[index] = indicesArray[index].GetInt();
		}

		// Getting the position you've set in Blender
		auto position =(*iter)["position"].GetArray();
		mesh->position = glm::vec3(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
		meshes.push_back(mesh);
	}
	return meshes;
			
}
