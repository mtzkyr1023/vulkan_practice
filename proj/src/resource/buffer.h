#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <vulkan/vulkan.hpp>

class Buffer
{
public:
	Buffer() {}
	virtual ~Buffer() {}

	void setupVertexBuffer(class RenderEngine* engine, size_t stride, size_t count, uint8_t* src);
	void setupIndexBuffer(class RenderEngine* engine, size_t stride, size_t count, uint8_t* src);
	void setupUniformBuffer(class RenderEngine* engine, size_t size, size_t count);

	void release(class RenderEngine* engine);

	const vk::Buffer& buffer(uint32_t index) { return buffers_[index]; }
	const vk::BufferView& view(uint32_t index) { return views_[index]; }

	const std::shared_ptr<class Memory>& memory() { return memory_; }

protected:

	std::shared_ptr<class Memory> memory_;
	
	std::vector<vk::Buffer> buffers_;
	std::vector<vk::BufferView> views_;
};

#endif