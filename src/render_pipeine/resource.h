#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <vulkan/vulkan.hpp>
#include <unordered_map>

class Resource {
public:
	Resource() :
		isAllocated_(false) {}
	virtual ~Resource() {}

	bool isAllocated() { return isAllocated_; }

protected:
	bool isAllocated_;
};

class Memory : public Resource {
public:
	Memory() {}
	~Memory() {}

	void allocateByBuffer(class RenderEngine* engine, vk::Buffer buffer, vk::MemoryPropertyFlags flags);
	void allocateByImage(class RenderEngine* engine, vk::Image image, vk::MemoryPropertyFlags flags);
	void free(class RenderEngine* engine);

	vk::DeviceMemory memory() { return memory_; }

private:
	vk::DeviceMemory memory_;
};

class Buffer : public Resource {
public:
	Buffer() {}
	~Buffer() {}

	void createUniformBuffer(class RenderEngine* engine, size_t size);
	void createVertexBuffer(class RenderEngine* engine, size_t stride, size_t elementCount);
	void createIndexBuffer(class RenderEngine* engine, size_t elementCount);
	void createCopyBuffer(class RenderEngine* engine, size_t size);
	void createStorageBuffer(class RenderEngine* engine, size_t stride, size_t elementCount);
	void cleanup(class RenderEngine* engine);

	vk::Buffer buffer() { return buffer_; }
	vk::BufferView view() { return view_; }

private:
	vk::Buffer buffer_;
	vk::BufferView view_;
};

class Image : public Resource {
public:
	Image() {}
	~Image() {}

	void createImage2D(
		class RenderEngine* engine,
		vk::Format format,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevel,
		vk::SampleCountFlagBits sampleCount);
	void cleanup(class RenderEngine* engine);

	vk::Image image() { return image_; }
	vk::ImageView view() { return view_; }

private:
	vk::Image image_;
	vk::ImageView view_;
};

using BufferSp = std::shared_ptr<Buffer>;
using ImageSp = std::shared_ptr<Image>;

class ResourceManager {
public:
	ResourceManager() {}
	~ResourceManager() {}

	BufferSp getBuffer(uint32_t index);
	ImageSp getImage(uint32_t index);

private:
	std::unordered_map<uint32_t, BufferSp> buffers_;
	std::unordered_map<uint32_t, ImageSp> images_;
};

#endif