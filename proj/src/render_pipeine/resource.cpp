#include "resource.h"
#include "../render_engine.h"

void Memory::allocateForBuffer(
	const vk::PhysicalDevice& physicalDevice,
	const vk::Device& device,
	const vk::BufferCreateInfo& info,
	vk::MemoryPropertyFlagBits propFlag)
{
#if 0
	vk::DeviceBufferMemoryRequirements deviceMemReqs = vk::DeviceBufferMemoryRequirements()
		.setPCreateInfo(&info);
	vk::MemoryRequirements2 memReqs = device.getBufferMemoryRequirements(deviceMemReqs);
	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.memoryRequirements.size)
		.setMemoryTypeIndex(getMemoryTypeIndex(physicalDevice, memReqs.memoryRequirements.memoryTypeBits, propFlag));
#else
	vk::Buffer buffer = device.createBuffer(info);

	vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(buffer);
	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.size)
		.setMemoryTypeIndex(getMemoryTypeIndex(physicalDevice, memReqs.memoryTypeBits, propFlag));

	alignment_ = memReqs.alignment;

	device.destroyBuffer(buffer);
#endif
	memory_ = device.allocateMemory(allocInfo);
}

void Memory::allocateForImage(
	const vk::PhysicalDevice& physicalDevice,
	const vk::Device& device,
	const vk::ImageCreateInfo& info,
	vk::MemoryPropertyFlagBits propFlag)
{
#if 0
	vk::DeviceImageMemoryRequirements deviceMemReqs = vk::DeviceImageMemoryRequirements()
		.setPCreateInfo(&info);
	vk::MemoryRequirements2 memReqs = device.getImageMemoryRequirements(deviceMemReqs);
	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.memoryRequirements.size)
		.setMemoryTypeIndex(getMemoryTypeIndex(physicalDevice, memReqs.memoryRequirements.memoryTypeBits, propFlag));
#else
	vk::Image image = device.createImage(info);

	vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(image);
	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.size)
		.setMemoryTypeIndex(getMemoryTypeIndex(physicalDevice, memReqs.memoryTypeBits, propFlag));

	alignment_ = memReqs.alignment;

	device.destroyImage(image);
#endif

	memory_ = device.allocateMemory(allocInfo);
}

void Memory::free(const vk::Device& device) {
	device.freeMemory(memory_);
}

void Memory::bind(const vk::Device& device, const vk::Buffer& buffer, vk::DeviceSize offset)
{
	device.bindBufferMemory(buffer, memory_, offset);
}

void Memory::bind(const vk::Device& device, const vk::Image& image, vk::DeviceSize offset)
{
	device.bindImageMemory(image, memory_, offset);
}


uint8_t* Memory::map(const vk::Device& device, vk::DeviceSize offset, vk::DeviceSize size)
{
	return (uint8_t*)device.mapMemory(memory_, offset, size);
}

void Memory::unmap(const vk::Device& device)
{
	device.unmapMemory(memory_);
}

uint32_t Memory::getMemoryTypeIndex(
	const vk::PhysicalDevice& physicalDevice,
	uint32_t bits,
	const vk::MemoryPropertyFlags& properties) {
	uint32_t result = 0;
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < 32; i++) {
		if ((bits & 1) == 1) {
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		bits >>= 1;
	}

	return 0xffffffff;
}


