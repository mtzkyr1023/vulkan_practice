#include "mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/GltfMaterial.h"
#include "assimp/vector3.h"
#include "glm/glm.hpp"

#include "../render_engine.h"
#include "../render_pipeine/resource.h"


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

	struct Vertex
	{
		glm::vec4 pos;
		glm::vec3 nor;
		glm::vec3 tan;
		glm::vec2 tex;
	};

	std::vector<Vertex> verticies;
	std::vector<uint32_t> indicies;

	vertexMemory_ = std::make_shared<Memory>();
	indexMemory_ = std::make_shared<Memory>();

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
			mat = std::make_shared<Material>(false);
		}
		else
		{
			mat = std::make_shared<Material>(true);
		}

		//mat->loadImage(engine, albedoFullPath.c_str(), normalFullPath.c_str(), pbrFullPath.c_str(), false);

		materials_.push_back(mat);
	}

	uint32_t vertexOffset = 0;
	for (uint32_t i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		Vertex vertex;

		for (uint32_t j = 0; j < mesh->mNumVertices; j++)
		{
			vertex.pos = glm::vec4(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, 1.0f);
			vertex.nor = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
			vertex.tan = glm::vec3(mesh->mBitangents[j].x, mesh->mBitangents[j].y, mesh->mBitangents[j].z);
			vertex.tex = glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
			verticies.push_back(vertex);
		}

		vertexCounts_.push_back(mesh->mNumVertices);

		for (uint32_t j = 0; j < mesh->mNumFaces; j++)
		{
			indicies.push_back(mesh->mFaces[j].mIndices[0] + vertexOffset);
			indicies.push_back(mesh->mFaces[j].mIndices[1] + vertexOffset);
			indicies.push_back(mesh->mFaces[j].mIndices[2] + vertexOffset);
		}

		indexCounts_.push_back(mesh->mNumFaces * 3);

		materials_[mesh->mMaterialIndex]->AddDrawInfo(indexCount_, mesh->mNumFaces * 3);

		indexCount_ += mesh->mNumFaces * 3;
		vertexOffset += mesh->mNumVertices;
	}

	vk::BufferCreateInfo vertexBufferCreateInfo = vk::BufferCreateInfo()
		.setSize(sizeof(Vertex) * verticies.size())
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);

	vk::BufferCreateInfo indexBufferCreateInfo = vk::BufferCreateInfo()
		.setSize(sizeof(uint32_t) * indicies.size())
		.setUsage(vk::BufferUsageFlagBits::eIndexBuffer);

	vertexMemory_->allocateForBuffer(
		engine->physicalDevice(),
		engine->device(),
		vertexBufferCreateInfo,
		vk::MemoryPropertyFlagBits::eHostVisible);

	{
		uint8_t* mappedMemory = vertexMemory_->map(engine->device(), 0, sizeof(Vertex) * verticies.size());
		memcpy_s(mappedMemory, sizeof(Vertex) * verticies.size(), verticies.data(), sizeof(Vertex) * verticies.size());

		vertexMemory_->unmap(engine->device());
	}

	indexMemory_->allocateForBuffer(
		engine->physicalDevice(),
		engine->device(),
		indexBufferCreateInfo,
		vk::MemoryPropertyFlagBits::eHostVisible);

	{
		uint8_t* mappedMemory = indexMemory_->map(engine->device(), 0, sizeof(uint32_t) * indicies.size());
		memcpy_s(mappedMemory, sizeof(uint32_t) * indicies.size(), indicies.data(), sizeof(uint32_t) * indicies.size());

		indexMemory_->unmap(engine->device());
	}
	vertexBuffer_ = engine->device().createBuffer(vertexBufferCreateInfo);
	indexBuffer_ = engine->device().createBuffer(indexBufferCreateInfo);

	vertexMemory_->bind(engine->device(), vertexBuffer_, 0);
	indexMemory_->bind(engine->device(), indexBuffer_, 0);

	vk::BufferViewCreateInfo vertexBufferViewCreateInfo = vk::BufferViewCreateInfo()
		.setBuffer(vertexBuffer_)
		.setFormat(vk::Format::eUndefined)
		.setOffset(0)
		.setRange(sizeof(Vertex) * verticies.size());

	//vertexBufferView_ = engine->device().createBufferView(vertexBufferViewCreateInfo);

	vk::BufferViewCreateInfo indexBufferViewCreateInfo = vk::BufferViewCreateInfo()
		.setBuffer(indexBuffer_)
		.setFormat(vk::Format::eR32Uint)
		.setOffset(0)
		.setRange(sizeof(uint32_t) * indicies.size());

	//indexBufferView_ = engine->device().createBufferView(indexBufferViewCreateInfo);
}

void Mesh::release(RenderEngine* engine)
{
	engine->device().destroyBufferView(vertexBufferView_);
	engine->device().destroyBufferView(indexBufferView_);
	engine->device().destroyBuffer(vertexBuffer_);
	engine->device().destroyBuffer(indexBuffer_);
	vertexMemory_->free(engine->device());
	indexMemory_->free(engine->device());

	for (const auto& ite : materials_)
	{
		ite->release(engine);
	}
}