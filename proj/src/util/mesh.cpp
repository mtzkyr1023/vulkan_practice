#include "mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/GltfMaterial.h"
#include "assimp/vector3.h"
#include "glm/glm.hpp"

#include "../render_engine.h"
#include "../resource/memory.h"
#include "../resource/buffer.h"


#include "material.h"

Mesh::Mesh() :
	indexCount_(0)
{

}

Mesh::~Mesh()
{

}


void Mesh::loadMesh(RenderEngine* engine, const char* foldername, const char* filename)
{
	Assimp::Importer* importer = new Assimp::Importer();
	uint32_t flags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate;

	std::string path = foldername;
	path += filename;

	const aiScene* scene = importer->ReadFile(path.c_str(), flags);

	for (uint32_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];
		
		bool isTransparent = true;
		aiString output;
		material->Get(AI_MATKEY_GLTF_ALPHAMODE, output);

		aiString albedoPath, normalPath, pbrPath;
		material->GetTexture(aiTextureType_BASE_COLOR, 0, &albedoPath);
		material->GetTexture(aiTextureType_NORMALS, 0, &normalPath);
		material->GetTexture(aiTextureType_METALNESS, 0, &pbrPath);

		std::string albedoFullPath = foldername;
		std::string normalFullPath = foldername;
		std::string pbrFullPath = foldername;

		albedoFullPath += albedoPath.C_Str();
		normalFullPath += normalPath.C_Str();
		pbrFullPath += pbrPath.C_Str();

		std::shared_ptr<Material> mat;
		if (output == aiString("OPAQUE"))
		{
			mat = std::make_shared<Material>(EMaterialType::eOpaque);
		}
		else if (output == aiString("MASK"))
		{
			mat = std::make_shared<Material>(EMaterialType::eMask);
		}
		else
		{
			mat = std::make_shared<Material>(EMaterialType::eTransparent);
		}

		mat->loadImage(engine, albedoFullPath.c_str(), normalFullPath.c_str(), pbrFullPath.c_str(), false);

		materials_.push_back(mat);
	}

	aabbMin_ = glm::vec3(FLT_MAX);
	aabbMax_ = glm::vec3(-FLT_MAX);

	aiNode* node = scene->mRootNode;

	vertexOffset_ = 0;
	indexOffset_ = 0;

	verticies_.clear();
	indicies_.clear();

	recursiveNode(scene, node);

	center_ = (aabbMin_ + aabbMax_) * 0.5f;

	vertexBuffer_ = std::make_shared<Buffer>();
	indexBuffer_ = std::make_shared<Buffer>();

	vertexBuffer_->setupVertexBuffer(engine, sizeof(Vertex), verticies_.size(), (uint8_t*)verticies_.data());
	indexBuffer_->setupIndexBuffer(engine, sizeof(int), indicies_.size(), (uint8_t*)indicies_.data());
}

void Mesh::release(RenderEngine* engine)
{
	vertexBuffer_->release(engine);
	indexBuffer_->release(engine);

	for (const auto& ite : materials_)
	{
		ite->release(engine);
	}
}

void Mesh::recursiveNode(const aiScene* scene, aiNode* node)
{
	if (node == nullptr) return;
	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		Vertex vertex;

		for (uint32_t j = 0; j < mesh->mNumVertices; j++)
		{
			glm::mat4 offsetMatrix = glm::mat4(
				glm::vec4((float)node->mTransformation[0][0], (float)node->mTransformation[0][1], (float)node->mTransformation[0][2], (float)node->mTransformation[0][3]),
				glm::vec4((float)node->mTransformation[1][0], (float)node->mTransformation[1][1], (float)node->mTransformation[1][2], (float)node->mTransformation[1][3]),
				glm::vec4((float)node->mTransformation[2][0], (float)node->mTransformation[2][1], (float)node->mTransformation[2][2], (float)node->mTransformation[2][3]),
				glm::vec4((float)node->mTransformation[3][0], (float)node->mTransformation[3][1], (float)node->mTransformation[3][2], (float)node->mTransformation[3][3])
			);
			vertex.pos = glm::vec4(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, 1.0f) * offsetMatrix;
			vertex.nor = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z) * glm::mat3(offsetMatrix);
			vertex.tan = glm::vec3(mesh->mBitangents[j].x, mesh->mBitangents[j].y, mesh->mBitangents[j].z) * glm::mat3(offsetMatrix);
			vertex.tex = glm::vec2(mesh->mTextureCoords[0][j].x, 1.0f - mesh->mTextureCoords[0][j].y);
			verticies_.push_back(vertex);

			aabbMin_ = glm::min(aabbMin_, glm::vec3(vertex.pos));
			aabbMax_ = glm::max(aabbMax_, glm::vec3(vertex.pos));
		}

		vertexCounts_.push_back(mesh->mNumVertices);

		for (uint32_t j = 0; j < mesh->mNumFaces; j++)
		{
			indicies_.push_back(mesh->mFaces[j].mIndices[0] + vertexOffset_);
			indicies_.push_back(mesh->mFaces[j].mIndices[1] + vertexOffset_);
			indicies_.push_back(mesh->mFaces[j].mIndices[2] + vertexOffset_);
		}

		indexCounts_.push_back(mesh->mNumFaces * 3);

		materials_[mesh->mMaterialIndex]->addDrawInfo(indexCount_, mesh->mNumFaces * 3);

		indexCount_ += mesh->mNumFaces * 3;
		vertexOffset_ += mesh->mNumVertices;
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		recursiveNode(scene, node->mChildren[i]);
	}
}
