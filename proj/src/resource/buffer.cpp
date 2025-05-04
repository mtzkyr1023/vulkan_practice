#include "buffer.h"
#include "memory.h"

#include "../render_engine.h"
#include "../defines.h"

void Buffer::setupVertexBuffer(RenderEngine* engine, size_t stride, size_t count, uint8_t* src)
{
	vk::Buffer buffer;
	Memory tempMemory;
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(stride * count)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc);

		tempMemory.allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);
		buffer = engine->device().createBuffer(bufferCreateInfo);
	}
	{
		uint8_t* mappedMemory = tempMemory.map(engine->device(), 0, stride * count);
		memcpy_s(mappedMemory, stride * count, src, stride * count);
		tempMemory.unmap(engine->device());
	}

	tempMemory.bind(engine->device(), buffer, 0);

	buffers_.resize(1);

	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(stride * count)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
		memory_ = std::make_shared<Memory>();

		memory_->allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		buffers_[0] = engine->device().createBuffer(bufferCreateInfo);
	}

	memory_->bind(engine->device(), buffers_[0], 0);

	auto cb = engine->allocateCommandBuffer(1);

	cb[0].begin(vk::CommandBufferBeginInfo());

	vk::BufferCopy bufferCopy = vk::BufferCopy()
		.setSize(stride * count)
		.setDstOffset(0)
		.setSrcOffset(0);

	cb[0].copyBuffer(buffer, buffers_[0], {bufferCopy});

	cb[0].end();


	vk::SubmitInfo submitInfo = vk::SubmitInfo()
		.setCommandBuffers(cb);

	vk::Fence fence = engine->device().createFence(vk::FenceCreateInfo());

	engine->graphicsQueue().submit(submitInfo, fence);

	engine->device().waitForFences({ fence }, vk::True, kTimeOut);

	tempMemory.free(engine->device());

	engine->device().freeCommandBuffers(engine->commandPool(), cb);
	engine->device().destroyFence(fence);
	engine->device().destroyBuffer(buffer);

}

void Buffer::setupIndexBuffer(RenderEngine* engine, size_t stride, size_t count, uint8_t* src)
{
	vk::Buffer buffer;
	Memory tempMemory;
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(stride * count)
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc);

		tempMemory.allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);
		buffer = engine->device().createBuffer(bufferCreateInfo);
	}
	{
		uint8_t* mappedMemory = tempMemory.map(engine->device(), 0, stride * count);
		memcpy_s(mappedMemory, stride * count, src, stride * count);
		tempMemory.unmap(engine->device());
	}

	tempMemory.bind(engine->device(), buffer, 0);

	buffers_.resize(1);

	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(stride * count)
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
		memory_ = std::make_shared<Memory>();

		memory_->allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		buffers_[0] = engine->device().createBuffer(bufferCreateInfo);
	}

	memory_->bind(engine->device(), buffers_[0], 0);

	auto cb = engine->allocateCommandBuffer(1);

	cb[0].begin(vk::CommandBufferBeginInfo());

	vk::BufferCopy bufferCopy = vk::BufferCopy()
		.setSize(stride * count)
		.setDstOffset(0)
		.setSrcOffset(0);

	cb[0].copyBuffer(buffer, buffers_[0], {bufferCopy});

	cb[0].end();


	vk::SubmitInfo submitInfo = vk::SubmitInfo()
		.setCommandBuffers(cb);

	vk::Fence fence = engine->device().createFence(vk::FenceCreateInfo());

	engine->graphicsQueue().submit(submitInfo, fence);

	engine->device().waitForFences({ fence }, vk::True, kTimeOut);

	tempMemory.free(engine->device());

	engine->device().freeCommandBuffers(engine->commandPool(), cb);
	engine->device().destroyFence(fence);
	engine->device().destroyBuffer(buffer);

}

void Buffer::setupUniformBuffer(RenderEngine* engine, size_t size, size_t count)
{
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(size * count)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

		memory_ = std::make_shared<Memory>();
		memory_->allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);
	}

	buffers_.resize(count);

	for (size_t i = 0; i < count; i++)
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

		buffers_[i] = engine->device().createBuffer(bufferCreateInfo);

		memory_->bind(engine->device(), buffers_[i], i * size);
	}
}

void Buffer::release(RenderEngine* engine)
{
	for (const auto& ite : buffers_)
	engine->device().destroyBuffer(ite);
	memory_->free(engine->device());
}

void Buffer::update(RenderEngine* engine, size_t offset, size_t size, void* data)
{
	uint8_t* mappedMemory = memory_->map(engine->device(), offset, size);

	memcpy_s(mappedMemory, size, data, size);

	memory_->unmap(engine->device());
}
