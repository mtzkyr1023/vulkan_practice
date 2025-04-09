#ifndef _MESH_H_
#define _MESH_H_

#include "memory"
#include "vector"
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"


class Mesh
{
public:
	Mesh();
	~Mesh();

	void loadMesh(class RenderEngine* engine, const char* foldername, const char* filename);
	void release(class RenderEngine* engine);

	const vk::Buffer& vertexBuffer() { return vertexBuffer_; }
	const vk::Buffer& indexBuffer() { return indexBuffer_; }

	const vk::BufferView& vertexBufferView() { return vertexBufferView_; }
	const vk::BufferView& indexBufferView() { return indexBufferView_; }

	uint32_t materialCount() { return (uint32_t)materials_.size(); }
	std::shared_ptr<class Material> material(uint32_t index) { return materials_[index]; }

	uint32_t allIndexCount() { return indexCount_; }
	uint32_t vertexCount(uint32_t index) { return vertexCounts_[index]; }
	uint32_t indexCount(uint32_t index) { return indexCounts_[index]; }

	const glm::vec3& aabbMin() { return aabbMin_; }
	const glm::vec3& aabbMax() { return aabbMax_; }
	const glm::vec3& center() { return center_; }

protected:
	std::shared_ptr<class Memory> vertexMemory_;
	std::shared_ptr<class Memory> indexMemory_;

	vk::Buffer vertexBuffer_;
	vk::Buffer indexBuffer_;
	vk::BufferView vertexBufferView_;
	vk::BufferView indexBufferView_;

	std::vector<uint32_t> vertexCounts_;
	std::vector<uint32_t> indexCounts_;

	std::vector<std::shared_ptr<class Material>> materials_;

	glm::vec3 aabbMin_;
	glm::vec3 aabbMax_;
	glm::vec3 center_;

	uint32_t indexCount_;
};

#endif