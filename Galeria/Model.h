#pragma once
#include <vector>
#include<string>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "Mesh.h"
#include "Texture.h"
#include <glm.hpp>

class Model
{
public:
	Model();

	void LoadModel(const std::string& fileName);
	void RenderModel();
	void ClearModel();

	~Model();

	// New API for per-mesh rendering / animation
	size_t GetMeshCount() const;
	std::string GetMeshName(unsigned int index) const;
	void RenderMesh(unsigned int index) const;
	glm::vec3 GetMeshCenter(unsigned int index) const;

private:
	void LoadNode(aiNode* node, const aiScene* scene); //assimp
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);
	std::vector<Mesh*>MeshList;
	std::vector<Texture*>TextureList;
	std::vector<unsigned int>meshTotex;
	// store the aiNode name associated with each pushed mesh to allow targeting parts
	std::vector<std::string> meshNames;
	// store mesh centers (in model space) computed from vertices
	std::vector<glm::vec3> meshCenters;
};

