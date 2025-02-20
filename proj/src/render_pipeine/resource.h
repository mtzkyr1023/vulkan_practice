#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <vulkan/vulkan.hpp>
#include <unordered_map>

class Memory {
public:
	Memory() :
		alignment_(0)
	{}
	~Memory() {}

	void allocateForBuffer(
		const vk::PhysicalDevice& physicalDevice,
		const vk::Device& device,
		const vk::BufferCreateInfo& info,
		vk::MemoryPropertyFlagBits propFlag);
	void allocateForImage(
		const vk::PhysicalDevice& physicalDevice,
		const vk::Device& device,
		const vk::ImageCreateInfo& info,
		vk::MemoryPropertyFlagBits propFlag
	);
	void free(const vk::Device& device);

	void bind(const vk::Device& device, const vk::Buffer& buffer, vk::DeviceSize offset);
	void bind(const vk::Device& device, const vk::Image& image, vk::DeviceSize offset);

	uint8_t* map(const vk::Device& device, vk::DeviceSize offset, vk::DeviceSize size);
	void unmap(const vk::Device& device);

	vk::DeviceMemory memory() { return memory_; }

	vk::DeviceSize alignment() { return alignment_; }

private:
	uint32_t getMemoryTypeIndex(
		const vk::PhysicalDevice& physicalDevice,
		uint32_t bits,
		const vk::MemoryPropertyFlags& properties);

private:
	vk::DeviceMemory memory_;

	vk::DeviceSize alignment_;
};

#endif