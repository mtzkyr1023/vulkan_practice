
#define STB_IMAGE_IMPLEMENTATION


#include "material.h"

#include "../render_engine.h"
#include "../render_pipeine/resource.h"

#include "../defines.h"

#include "stb_image.h"



Material::Material(bool isTransparent) :
	isTransparent_(isTransparent)
{

}

Material::~Material()
{

}

void Material::loadImage(
	RenderEngine* engine,
	const char* albedoFilename,
	const char* normalFilename,
	const char* pbrFilename,
	bool generateMipmap)
{
	unsigned char* albedo = nullptr;
	unsigned char* normal = nullptr;
	unsigned char* pbr = nullptr;
	int bpp;
	int albedoWidth, albedoHeight;
	int normalWidth, normalHeight;
	int pbrWidth, pbrHeight;
	albedo = stbi_load(albedoFilename, &albedoWidth, &albedoHeight, &bpp, 4);
	normal = stbi_load(normalFilename, &normalWidth, &normalHeight, &bpp, 4);
	pbr = stbi_load(pbrFilename, &pbrWidth, &pbrHeight, &bpp, 4);

	int width = std::max(albedoWidth, std::max(normalWidth, pbrWidth));
	int height = std::max(albedoHeight, std::max(normalHeight, pbrHeight));

	if (width < 0 || height < 0)
	{
		width = 4;
		height = 4;
	}


	Memory tempMemory;
	vk::Buffer buffer;


	vk::DeviceSize size = 0;
	vk::DeviceSize alignment = 0;
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(3)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		memory_ = std::make_shared<Memory>();

		memory_->allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::DeviceSize a = memory_->alignment();
		size = memory_->size();
		alignment = width * height * sizeof(uint32_t) + (memory_->alignment() - 1) & ~(memory_->alignment() - 1);
	}

	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(alignment * (vk::DeviceSize)ETextureType::eNum)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		tempMemory.allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);

		uint8_t* mappedMemory = tempMemory.map(engine->device(), 0, alignment);
		if (albedo != nullptr)
		{
			memcpy_s(mappedMemory + alignment * (size_t)ETextureType::eAlbedo, alignment, albedo, alignment);
		}
		if (normal != nullptr)
		{
			memcpy_s(mappedMemory + alignment * (size_t)ETextureType::eNormal, alignment, normal, alignment);
		}
		if (pbr != nullptr)
		{
			memcpy_s(mappedMemory + alignment * (size_t)ETextureType::ePBR, alignment, pbr, alignment);
		}

		tempMemory.unmap(engine->device());

		buffer = engine->device().createBuffer(bufferCreateInfo);

		tempMemory.bind(engine->device(), buffer, 0);
	}

	{
		uint32_t type = (uint32_t)ETextureType::eAlbedo;
		vk::Image& image = images_[type];
		vk::ImageView& view = imageViews_[type];

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, alignment * (vk::DeviceSize)type);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	{
		uint32_t type = (uint32_t)ETextureType::eNormal;
		vk::Image& image = images_[type];
		vk::ImageView& view = imageViews_[type];

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, alignment * (vk::DeviceSize)type);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	{
		uint32_t type = (uint32_t)ETextureType::ePBR;
		vk::Image& image = images_[type];
		vk::ImageView& view = imageViews_[type];

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, alignment * (vk::DeviceSize)type);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	std::vector<vk::CommandBuffer> cbs = engine->allocateCommandBuffer(1);

	cbs[0].begin(vk::CommandBufferBeginInfo());

	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 3> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setImage(images_[(uint32_t)ETextureType::eAlbedo]);
		barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[0].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[1].setImage(images_[(uint32_t)ETextureType::eNormal]);
		barriers[1].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[1].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[2].setImage(images_[(uint32_t)ETextureType::ePBR]);
		barriers[2].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[2].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		cbs[0].pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::DependencyFlagBits::eDeviceGroup,
			nullptr,
			nullptr,
			barriers);
	}

	{
		vk::BufferImageCopy copyInfo = vk::BufferImageCopy()
			.setBufferOffset(alignment * (size_t)ETextureType::eAlbedo)
			.setBufferImageHeight(height * sizeof(uint32_t))
			.setBufferRowLength(width)
			.setImageExtent(vk::Extent3D(width, height, 1))
			.setImageOffset(0)
			.setImageSubresource(
				vk::ImageSubresourceLayers()
				.setMipLevel(0)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0));
		cbs[0].copyBufferToImage(buffer, images_[(uint32_t)ETextureType::eAlbedo], vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}
	{
		vk::BufferImageCopy copyInfo = vk::BufferImageCopy()
			.setBufferOffset(alignment * (size_t)ETextureType::eNormal)
			.setBufferImageHeight(height * sizeof(uint32_t))
			.setBufferRowLength(width)
			.setImageExtent(vk::Extent3D(width, height, 1))
			.setImageOffset(0)
			.setImageSubresource(
				vk::ImageSubresourceLayers()
				.setMipLevel(0)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0));
		cbs[0].copyBufferToImage(buffer, images_[(uint32_t)ETextureType::eNormal], vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}
	{
		vk::BufferImageCopy copyInfo = vk::BufferImageCopy()
			.setBufferOffset(alignment * (size_t)ETextureType::ePBR)
			.setBufferImageHeight(height * sizeof(uint32_t))
			.setBufferRowLength(width)
			.setImageExtent(vk::Extent3D(width, height, 1))
			.setImageOffset(0)
			.setImageSubresource(
				vk::ImageSubresourceLayers()
				.setMipLevel(0)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0));
		cbs[0].copyBufferToImage(buffer, images_[(uint32_t)ETextureType::ePBR], vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}

	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 3> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(images_[(uint32_t)ETextureType::eAlbedo]);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[1].setImage(images_[(uint32_t)ETextureType::eNormal]);
		barriers[1].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[1].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[2].setImage(images_[(uint32_t)ETextureType::ePBR]);
		barriers[2].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[2].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		cbs[0].pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::DependencyFlagBits::eDeviceGroup,
			nullptr,
			nullptr,
			barriers);
	}

	cbs[0].end();

	vk::SubmitInfo submitInfo = vk::SubmitInfo()
		.setCommandBuffers(cbs);
	
	vk::Fence fence = engine->device().createFence(vk::FenceCreateInfo());

	engine->graphicsQueue().submit(submitInfo, fence);

	engine->device().waitForFences({ fence }, vk::True, kTimeOut);

	tempMemory.free(engine->device());

	engine->device().freeCommandBuffers(engine->commandPool(), cbs);
	engine->device().destroyFence(fence);
	engine->device().destroyBuffer(buffer);

	stbi_image_free(albedo);
	stbi_image_free(normal);
	stbi_image_free(pbr);
}

void Material::release(RenderEngine* engine)
{
	for (const auto& ite : images_)
	{
		engine->device().destroyImage(ite);
	}

	for (const auto& ite : imageViews_)
	{
		engine->device().destroyImageView(ite);
	}

	memory_->free(engine->device());
}