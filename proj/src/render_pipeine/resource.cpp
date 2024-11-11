#include "resource.h"
#include "../render_engine.h"

void Memory::allocateByBuffer(RenderEngine* engine, vk::Buffer buffer, vk::MemoryPropertyFlags flags) {
	vk::MemoryRequirements memReqs = engine->device().getBufferMemoryRequirements(buffer);
	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.size)
		.setMemoryTypeIndex(engine->getMemoryTypeIndex(memReqs.memoryTypeBits, flags));

	memory_ = engine->device().allocateMemory(allocInfo);

	isAllocated_ = true;
}


void Memory::allocateByImage(RenderEngine* engine, vk::Image image, vk::MemoryPropertyFlags flags) {
	vk::MemoryRequirements memReqs = engine->device().getImageMemoryRequirements(image);
	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.size)
		.setMemoryTypeIndex(engine->getMemoryTypeIndex(memReqs.memoryTypeBits, flags));
	
	memory_ = engine->device().allocateMemory(allocInfo);

	isAllocated_ = true;
}

void Memory::free(RenderEngine* engine) {
	engine->device().freeMemory(memory_);
	isAllocated_ = false;
}


void Buffer::createUniformBuffer(RenderEngine* engine, size_t size) {
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
		.setSize((vk::DeviceSize)size)
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

	buffer_ = engine->device().createBuffer(bufferCreateInfo);

	isAllocated_ = true;
}

void Buffer::createVertexBuffer(RenderEngine* engine, size_t stride, size_t elementCount) {
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
		.setSize((vk::DeviceSize)(stride * elementCount))
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

	buffer_ = engine->device().createBuffer(bufferCreateInfo);

	isAllocated_ = true;
}

void Buffer::createIndexBuffer(RenderEngine* engine, size_t elementCount) {
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
		.setSize((vk::DeviceSize)(sizeof(uint32_t) * elementCount))
		.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);

	buffer_ = engine->device().createBuffer(bufferCreateInfo);

	isAllocated_ = true;
}

void Buffer::createCopyBuffer(RenderEngine* engine, size_t size) {
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
		.setSize((vk::DeviceSize)size)
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

	buffer_ = engine->device().createBuffer(bufferCreateInfo);

	isAllocated_ = true;
}

void Buffer::createStorageBuffer(RenderEngine* engine, size_t stride, size_t elementCount) {
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
		.setSize((vk::DeviceSize)(stride * elementCount))
		.setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst);

	buffer_ = engine->device().createBuffer(bufferCreateInfo);

	isAllocated_ = true;
}

void Buffer::cleanup(RenderEngine* engine) {
	engine->device().destroyBuffer(buffer_);

	isAllocated_ = false;
}

void Image::createImage2D(
	RenderEngine* engine,
	vk::Format format,
	uint32_t width,
	uint32_t height,
	uint32_t mipLevel,
	vk::SampleCountFlagBits sampleCount) {
	vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
		.setFormat(format)
		.setImageType(vk::ImageType::e2D)
		.setExtent(vk::Extent3D(width, height, 1))
		.setSamples(sampleCount)
		.setMipLevels(mipLevel)
		.setArrayLayers(1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eColorAttachment);

	image_ = engine->device().createImage(imageCreateInfo);

	vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
		.setFormat(format)
		.setImage(image_)
		.setComponents(
			{
				vk::ComponentSwizzle::eR,
				vk::ComponentSwizzle::eG,
				vk::ComponentSwizzle::eB,
				vk::ComponentSwizzle::eA
			})
		.setSubresourceRange(
			vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1
			))
		.setViewType(vk::ImageViewType::e2D);

	view_ = engine->device().createImageView(viewCreateInfo);
}

void Image::cleanup(RenderEngine* engine) {
	engine->device().destroyImage(image_);
	engine->device().destroyImageView(view_);
}

std::vector<BufferSp> ResourceManager::getBuffer(uint32_t index, uint32_t num) {
	if (images_.find(index) == images_.end()) {
		std::vector<BufferSp> buffers;
		buffers.resize(num);
		for (uint32_t i = 0; i < num; i++) {
			buffers[i] = std::make_shared<Buffer>();
		}

		buffers_[index] = buffers;
	}

	return buffers_[index];
}

std::vector<ImageSp> ResourceManager::getImage(uint32_t index, uint32_t num) {
	if (images_.find(index) == images_.end()) {
		std::vector<ImageSp> images;
		images.resize(num);
		for (uint32_t i = 0; i < num; i++) {
			images[i] = std::make_shared<Image>();
		}

		images_[index] = images;
	}

	return images_[index];
}