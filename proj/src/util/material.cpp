
#define STB_IMAGE_IMPLEMENTATION


#include "material.h"

#include "../render_engine.h"
#include "../render_pipeine/resource.h"

#include "../defines.h"

#include "stb_image.h"



Material::Material(EMaterialType type) :
	materialType_(type)
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


	int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	std::array<Memory, (uint32_t)ETextureType::eNum> tempMemory;
	std::array<vk::Buffer, (uint32_t)ETextureType::eNum> buffer;


	vk::DeviceSize size = 0;
	vk::DeviceSize alignment = 0;
	for (uint32_t i = 0; i < (uint32_t)ETextureType::eNum; i++)
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(mipLevels)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		memory_.push_back(std::make_shared<Memory>());

		memory_[i]->allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::DeviceSize a = memory_[i]->alignment();
		size = memory_[i]->size();
		alignment = width * height * sizeof(uint32_t) + (memory_[i]->alignment() - 1) & ~(memory_[i]->alignment() - 1);
	}

	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		tempMemory[0].allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);
		tempMemory[1].allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);
		tempMemory[2].allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);

		{
			uint8_t* mappedMemory = tempMemory[0].map(engine->device(), 0, alignment);
			if (albedo != nullptr)
			{
				memcpy_s(mappedMemory, alignment, albedo, alignment);
			}
		}
		{
			uint8_t* mappedMemory = tempMemory[1].map(engine->device(), 0, alignment);
			if (normal != nullptr)
			{
				memcpy_s(mappedMemory, alignment, normal, alignment);
			}
		}
		{
			uint8_t* mappedMemory = tempMemory[2].map(engine->device(), 0, alignment);
			if (pbr != nullptr)
			{
				memcpy_s(mappedMemory, alignment, pbr, alignment);
			}
		}

		tempMemory[0].unmap(engine->device());
		tempMemory[1].unmap(engine->device());
		tempMemory[2].unmap(engine->device());

		buffer[0] = engine->device().createBuffer(bufferCreateInfo);
		buffer[1] = engine->device().createBuffer(bufferCreateInfo);
		buffer[2] = engine->device().createBuffer(bufferCreateInfo);

		tempMemory[0].bind(engine->device(), buffer[0], 0);
		tempMemory[1].bind(engine->device(), buffer[1], 0);
		tempMemory[2].bind(engine->device(), buffer[2], 0);
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
			.setMipLevels(mipLevels)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc);

		image = engine->device().createImage(imageCreateInfo);

		memory_[type]->bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(mipLevels)
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
			.setMipLevels(mipLevels)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc);

		image = engine->device().createImage(imageCreateInfo);

		memory_[type]->bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(mipLevels)
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
			.setMipLevels(mipLevels)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc);

		image = engine->device().createImage(imageCreateInfo);

		memory_[type]->bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(mipLevels)
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
			.setBufferOffset(0)
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
		cbs[0].copyBufferToImage(buffer[0], images_[(uint32_t)ETextureType::eAlbedo], vk::ImageLayout::eTransferDstOptimal, {copyInfo});
	}
	{
		vk::BufferImageCopy copyInfo = vk::BufferImageCopy()
			.setBufferOffset(0)
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
		cbs[0].copyBufferToImage(buffer[1], images_[(uint32_t)ETextureType::eNormal], vk::ImageLayout::eTransferDstOptimal, {copyInfo});
	}
	{
		vk::BufferImageCopy copyInfo = vk::BufferImageCopy()
			.setBufferOffset(0)
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
		cbs[0].copyBufferToImage(buffer[2], images_[(uint32_t)ETextureType::ePBR], vk::ImageLayout::eTransferDstOptimal, {copyInfo});
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
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setImage(images_[(uint32_t)ETextureType::eAlbedo]);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setImage(images_[(uint32_t)ETextureType::eNormal]);
		barriers[1].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[1].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[2].setImage(images_[(uint32_t)ETextureType::ePBR]);
		barriers[2].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[2].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
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

	int mipWidth = width;
	int mipHeight = height;

	for (int i = 1; i < mipLevels; i++)
	{
		{

			vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(i)
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

		for (uint32_t j = 0; j < (uint32_t)ETextureType::eNum; j++)
		{
			int dstMipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
			int dstMipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
			vk::ImageBlit blit = vk::ImageBlit()
				.setSrcOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth, mipHeight, 1) })
				.setSrcSubresource(
					vk::ImageSubresourceLayers()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setMipLevel(i - 1)
					.setBaseArrayLayer(0)
					.setLayerCount(1))
				.setDstOffsets({
					vk::Offset3D(0, 0, 0),
					vk::Offset3D(dstMipWidth, dstMipHeight, 1)})
				.setDstSubresource(
					vk::ImageSubresourceLayers()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setMipLevel(i)
					.setBaseArrayLayer(0)
					.setLayerCount(1));

			cbs[0].blitImage(
				images_[j],
				vk::ImageLayout::eTransferSrcOptimal,
				images_[j],
				vk::ImageLayout::eTransferDstOptimal,
				1, &blit, vk::Filter::eLinear);
		}


		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(i)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 3> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setImage(images_[(uint32_t)ETextureType::eAlbedo]);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setImage(images_[(uint32_t)ETextureType::eNormal]);
		barriers[1].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[1].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[2].setImage(images_[(uint32_t)ETextureType::ePBR]);
		barriers[2].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[2].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
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

		mipWidth >>= 1;
		mipHeight >>= 1;
	}

	for (int i = 0; i < mipLevels; i++)
	{

		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(i)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 3> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(images_[(uint32_t)ETextureType::eAlbedo]);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[1].setImage(images_[(uint32_t)ETextureType::eNormal]);
		barriers[1].setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[1].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[2].setImage(images_[(uint32_t)ETextureType::ePBR]);
		barriers[2].setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
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

	tempMemory[0].free(engine->device());
	tempMemory[1].free(engine->device());
	tempMemory[2].free(engine->device());

	engine->device().freeCommandBuffers(engine->commandPool(), cbs);
	engine->device().destroyFence(fence);
	engine->device().destroyBuffer(buffer[0]);
	engine->device().destroyBuffer(buffer[1]);
	engine->device().destroyBuffer(buffer[2]);

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

	for (auto& ite : memory_)
	{
		ite->free(engine->device());
	}
}