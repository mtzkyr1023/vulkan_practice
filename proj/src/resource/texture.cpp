
#define STB_IMAGE_IMPLEMENTATION


#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>

#include <glm/gtc/reciprocal.hpp>

#include "texture.h"
#include "memory.h"
#include "../render_engine.h"

#include "../defines.h"

#include "../util/stb_image.h"

void Texture::setupRenderTarget2d(RenderEngine* engine, uint32_t width, uint32_t height, vk::Format format)
{
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(format)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eInputAttachment);

		memory_ = std::make_shared<Memory>();
		memory_->allocateForImage(
			engine->physicalDevice(),
			engine->device(),
			imageCreateInfo,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	}


	{
		vk::Image& image = image_;
		vk::ImageView& view = view_;

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(format)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eInputAttachment);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(format)
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
}

void Texture::setupDepthStencilBuffer(RenderEngine* engine, uint32_t width, uint32_t height)
{

	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eDepthStencilAttachment |
				vk::ImageUsageFlagBits::eInputAttachment);

		memory_ = std::make_shared<Memory>();
		memory_->allocateForImage(
			engine->physicalDevice(),
			engine->device(),
			imageCreateInfo,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	}


	{
		vk::Image& image = image_;
		vk::ImageView& view = view_;

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(1)
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eDepthStencilAttachment |
				vk::ImageUsageFlagBits::eInputAttachment);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eDepth))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}
}

void Texture::setupResource2d(RenderEngine* engine, const char* filename)
{
	unsigned char* pixels = nullptr;
	int bpp;
	int width, height;
	pixels = stbi_load(filename, &width, &height, &bpp, 4);

	if (width < 0 || height < 0)
	{
		width = 4;
		height = 4;
	}


	int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	Memory tempMemory;
	vk::Buffer buffer;

	vk::DeviceSize size = 0;
	vk::DeviceSize alignment = 0;
	
	memory_ = std::make_shared<Memory>();

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


		memory_->allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::DeviceSize a = memory_->alignment();
		size = memory_->size();
		alignment = width * height * sizeof(uint32_t) + (memory_->alignment() - 1) & ~(memory_->alignment() - 1);
	}

	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		tempMemory.allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);

		{
			uint8_t* mappedMemory = tempMemory.map(engine->device(), 0, alignment);
			if (pixels != nullptr)
			{
				memcpy_s(mappedMemory, alignment, pixels, alignment);
			}
		}

		tempMemory.unmap(engine->device());

		buffer = engine->device().createBuffer(bufferCreateInfo);

		tempMemory.bind(engine->device(), buffer, 0);
	}

	{
		vk::Image& image = image_;
		vk::ImageView& view = view_;

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

		memory_->bind(engine->device(), image, 0);

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

		std::array<vk::ImageMemoryBarrier, 1> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setImage(image_);
		barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[0].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

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
		cbs[0].copyBufferToImage(buffer, image_, vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}

	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 1> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setImage(image_);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

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

			std::array<vk::ImageMemoryBarrier, 1> barriers;
			barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setImage(image_);
			barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
			barriers[0].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
			barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setSubresourceRange(colorSubresourceRange);

			cbs[0].pipelineBarrier(
				vk::PipelineStageFlagBits::eAllGraphics,
				vk::PipelineStageFlagBits::eAllGraphics,
				vk::DependencyFlagBits::eDeviceGroup,
				nullptr,
				nullptr,
				barriers);
		}

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
					vk::Offset3D(dstMipWidth, dstMipHeight, 1) })
					.setDstSubresource(
						vk::ImageSubresourceLayers()
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setMipLevel(i)
						.setBaseArrayLayer(0)
						.setLayerCount(1));

			cbs[0].blitImage(
				image_,
				vk::ImageLayout::eTransferSrcOptimal,
				image_,
				vk::ImageLayout::eTransferDstOptimal,
				1, &blit, vk::Filter::eLinear);
		}


		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(i)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 1> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setImage(image_);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

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

		std::array<vk::ImageMemoryBarrier, 1> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(image_);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

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

	stbi_image_free(pixels);
}

void Texture::setupResourceCubemap(RenderEngine* engine, const char* filename)
{
	enum EAxisType
	{
		eRight = 0,
		eLeft,
		eUp,
		eDown,
		eForward,
		eBack,
		
		eNum,
	};


	float* pixels = nullptr;
	int bpp;
	int srcWidth, srcHeight;
	pixels = stbi_loadf(filename, &srcWidth, &srcHeight, &bpp, 4);

	int width = srcWidth / 4;
	int height = width;

	std::vector<float> tempMap[6];
	for (int i = 0; i < 6; i++)
	{
		tempMap[i].resize(width * height * 4);
	}

	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				glm::vec3 axis = glm::vec3(1.0f, (float)y / (float)height - (float)(height / 2), -((float)x / (float)width - (float)(width / 2)));
				axis = glm::normalize(axis);

				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 side = glm::vec3(-1.0f, 0.0f, 0.0f);

				float dotProductSide = glm::dot(side, axis);
				float dotProductUp = glm::dot(up, axis);

				float theta = glm::cot(dotProductSide) + glm::pi<float>();
				float phi = glm::cot(dotProductUp);

				float normalizedX = theta / glm::two_pi<float>();
				float normalizedY = phi / glm::pi<float>();

				int sampledX = glm::clamp((int)(normalizedX * (float)srcWidth), 0, width - 1);
				int sampledY = glm::clamp((int)(normalizedY * (float)srcHeight), 0, height - 1);

				memcpy_s(&tempMap[0][y * width + x], sizeof(float) * 4, &pixels[sampledY * srcWidth + sampledX], sizeof(float) * 4);
			}
		}
	}
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				glm::vec3 axis = glm::vec3(-1.0f, (float)y / (float)height - (float)(height / 2), (float)x / (float)width - (float)(width / 2));
				axis = glm::normalize(axis);

				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 side = glm::vec3(-1.0f, 0.0f, 0.0f);

				float dotProductSide = glm::dot(side, axis);
				float dotProductUp = glm::dot(up, axis);

				float theta = glm::cot(dotProductSide) + glm::pi<float>();
				float phi = glm::cot(dotProductUp);

				float normalizedX = theta / glm::two_pi<float>();
				float normalizedY = phi / glm::pi<float>();

				int sampledX = glm::clamp((int)(normalizedX * (float)srcWidth), 0, width - 1);
				int sampledY = glm::clamp((int)(normalizedY * (float)srcHeight), 0, height - 1);

				memcpy_s(&tempMap[1][y * width + x], sizeof(float) * 4, &pixels[sampledY * srcWidth + sampledX], sizeof(float) * 4);
			}
		}
	}
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				glm::vec3 axis = glm::vec3(-((float)x / (float)width - (float)(width / 2)), 1.0f, -((float)y / (float)height - (float)(height / 2)));
				axis = glm::normalize(axis);

				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 side = glm::vec3(-1.0f, 0.0f, 0.0f);

				float dotProductSide = glm::dot(side, axis);
				float dotProductUp = glm::dot(up, axis);

				float theta = glm::cot(dotProductSide) + glm::pi<float>();
				float phi = glm::cot(dotProductUp);

				float normalizedX = theta / glm::two_pi<float>();
				float normalizedY = phi / glm::pi<float>();

				int sampledX = glm::clamp((int)(normalizedX * (float)srcWidth), 0, width - 1);
				int sampledY = glm::clamp((int)(normalizedY * (float)srcHeight), 0, height - 1);

				memcpy_s(&tempMap[2][y * width + x], sizeof(float) * 4, &pixels[sampledY * srcWidth + sampledX], sizeof(float) * 4);
			}
		}
	}
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				glm::vec3 axis = glm::vec3(((float)x / (float)width - (float)(width / 2)), -1.0f, ((float)y / (float)height - (float)(height / 2)));
				axis = glm::normalize(axis);

				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 side = glm::vec3(-1.0f, 0.0f, 0.0f);

				float dotProductSide = glm::dot(side, axis);
				float dotProductUp = glm::dot(up, axis);

				float theta = glm::cot(dotProductSide) + glm::pi<float>();
				float phi = glm::cot(dotProductUp);

				float normalizedX = theta / glm::two_pi<float>();
				float normalizedY = phi / glm::pi<float>();

				int sampledX = glm::clamp((int)(normalizedX * (float)srcWidth), 0, width - 1);
				int sampledY = glm::clamp((int)(normalizedY * (float)srcHeight), 0, height - 1);

				memcpy_s(&tempMap[3][y * width + x], sizeof(float) * 4, &pixels[sampledY * srcWidth + sampledX], sizeof(float) * 4);
			}
		}
	}
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				glm::vec3 axis = glm::vec3((float)x / (float)width - (float)(width / 2), -((float)y / (float)height - (float)(height / 2)), 1.0f);
				axis = glm::normalize(axis);

				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 side = glm::vec3(-1.0f, 0.0f, 0.0f);

				float dotProductSide = glm::dot(side, axis);
				float dotProductUp = glm::dot(up, axis);

				float theta = glm::cot(dotProductSide) + glm::pi<float>();
				float phi = glm::cot(dotProductUp);

				float normalizedX = theta / glm::two_pi<float>();
				float normalizedY = phi / glm::pi<float>();

				int sampledX = glm::clamp((int)(normalizedX * (float)srcWidth), 0, width - 1);
				int sampledY = glm::clamp((int)(normalizedY * (float)srcHeight), 0, height - 1);

				memcpy_s(&tempMap[4][y * width + x], sizeof(float) * 4, &pixels[sampledY * srcWidth + sampledX], sizeof(float) * 4);
			}
		}
	}
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				glm::vec3 axis = glm::vec3(-((float)x / (float)width - (float)(width / 2)), -((float)y / (float)height - (float)(height / 2)), 1.0f);
				axis = glm::normalize(axis);

				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 side = glm::vec3(-1.0f, 0.0f, 0.0f);

				float dotProductSide = glm::dot(side, axis);
				float dotProductUp = glm::dot(up, axis);

				float theta = glm::cot(dotProductSide) + glm::pi<float>();
				float phi = glm::cot(dotProductUp);

				float normalizedX = theta / glm::two_pi<float>();
				float normalizedY = phi / glm::pi<float>();

				int sampledX = glm::clamp((int)(normalizedX * (float)srcWidth), 0, width - 1);
				int sampledY = glm::clamp((int)(normalizedY * (float)srcHeight), 0, height - 1);

				memcpy_s(&tempMap[5][y * width + x], sizeof(float) * 4, &pixels[sampledY * srcWidth + sampledX], sizeof(float) * 4);
			}
		}
	}

	int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	Memory tempMemory;
	vk::Buffer buffer;

	vk::DeviceSize size = 0;
	vk::DeviceSize alignment = 0;

	memory_ = std::make_shared<Memory>();

	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setFlags(vk::ImageCreateFlagBits::eCubeCompatible)
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(6)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(mipLevels)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);


		memory_->allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::DeviceSize a = memory_->alignment();
		size = memory_->size();
		alignment = width * height * sizeof(uint32_t) * 4 + (memory_->alignment() - 1) & ~(memory_->alignment() - 1);
	}

	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		tempMemory.allocateForBuffer(engine->physicalDevice(), engine->device(), bufferCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);

		{
			uint8_t* mappedMemory = tempMemory.map(engine->device(), 0, alignment);
			if (pixels != nullptr)
			{
				for (int i = 0; i < 6; i++)
				{
					memcpy_s(mappedMemory + sizeof(float) * width * height * 4 * i, sizeof(float) * width * height * 4, tempMap[i].data(), sizeof(float) * width * height * 4);
				}
			}
		}

		tempMemory.unmap(engine->device());

		buffer = engine->device().createBuffer(bufferCreateInfo);

		tempMemory.bind(engine->device(), buffer, 0);
	}

	{
		vk::Image& image = image_;
		vk::ImageView& view = view_;

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setFlags(vk::ImageCreateFlagBits::eCubeCompatible)
			.setExtent(vk::Extent3D(width, height, 1))
			.setArrayLayers(6)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setMipLevels(mipLevels)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setBaseMipLevel(0)
				.setLevelCount(mipLevels)
				.setLayerCount(6)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::eCube)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	std::vector<vk::CommandBuffer> cbs = engine->allocateCommandBuffer(1);

	cbs[0].begin(vk::CommandBufferBeginInfo());

	for (int i = 0; i < 6; i++)
	{
		{
			vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(i)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

			std::array<vk::ImageMemoryBarrier, 1> barriers;
			barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setImage(image_);
			barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
			barriers[0].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
			barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setSubresourceRange(colorSubresourceRange);

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
				.setBufferOffset(sizeof(float) * width * height * 4 * i)
				.setBufferImageHeight(height * sizeof(uint32_t) * 4)
				.setBufferRowLength(width)
				.setImageExtent(vk::Extent3D(width, height, 1))
				.setImageOffset(0)
				.setImageSubresource(
					vk::ImageSubresourceLayers()
					.setMipLevel(0)
					.setLayerCount(1)
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseArrayLayer(0));
			cbs[0].copyBufferToImage(buffer, image_, vk::ImageLayout::eTransferDstOptimal, { copyInfo });
		}

		{
			vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(i)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

			std::array<vk::ImageMemoryBarrier, 1> barriers;
			barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
			barriers[0].setImage(image_);
			barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
			barriers[0].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
			barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setSubresourceRange(colorSubresourceRange);

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

		for (int j = 1; j < mipLevels; j++)
		{
			{

				vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseArrayLayer(i)
					.setBaseMipLevel(j)
					.setLayerCount(1)
					.setLevelCount(1);

				std::array<vk::ImageMemoryBarrier, 1> barriers;
				barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
				barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
				barriers[0].setImage(image_);
				barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
				barriers[0].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
				barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
				barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
				barriers[0].setSubresourceRange(colorSubresourceRange);

				cbs[0].pipelineBarrier(
					vk::PipelineStageFlagBits::eAllGraphics,
					vk::PipelineStageFlagBits::eAllGraphics,
					vk::DependencyFlagBits::eDeviceGroup,
					nullptr,
					nullptr,
					barriers);
			}

			{
				int dstMipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
				int dstMipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
				vk::ImageBlit blit = vk::ImageBlit()
					.setSrcOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth, mipHeight, 1) })
					.setSrcSubresource(
						vk::ImageSubresourceLayers()
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setMipLevel(j - 1)
						.setBaseArrayLayer(i)
						.setLayerCount(1))
					.setDstOffsets({
						vk::Offset3D(0, 0, 0),
						vk::Offset3D(dstMipWidth, dstMipHeight, 1) })
						.setDstSubresource(
							vk::ImageSubresourceLayers()
							.setAspectMask(vk::ImageAspectFlagBits::eColor)
							.setMipLevel(j)
							.setBaseArrayLayer(i)
							.setLayerCount(1));

				cbs[0].blitImage(
					image_,
					vk::ImageLayout::eTransferSrcOptimal,
					image_,
					vk::ImageLayout::eTransferDstOptimal,
					1, &blit, vk::Filter::eLinear);
			}


			vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(i)
				.setBaseMipLevel(j)
				.setLayerCount(1)
				.setLevelCount(1);

			std::array<vk::ImageMemoryBarrier, 1> barriers;
			barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
			barriers[0].setImage(image_);
			barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
			barriers[0].setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
			barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setSubresourceRange(colorSubresourceRange);

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

		for (int j = 0; j < mipLevels; j++)
		{

			vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(i)
				.setBaseMipLevel(j)
				.setLayerCount(1)
				.setLevelCount(1);

			std::array<vk::ImageMemoryBarrier, 1> barriers;
			barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
			barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			barriers[0].setImage(image_);
			barriers[0].setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
			barriers[0].setNewLayout(vk::ImageLayout::eTransferDstOptimal);
			barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
			barriers[0].setSubresourceRange(colorSubresourceRange);

			cbs[0].pipelineBarrier(
				vk::PipelineStageFlagBits::eAllGraphics,
				vk::PipelineStageFlagBits::eAllGraphics,
				vk::DependencyFlagBits::eDeviceGroup,
				nullptr,
				nullptr,
				barriers);
		}
	}

	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(6)
			.setLevelCount(mipLevels);

		std::array<vk::ImageMemoryBarrier, 1> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(image_);
		barriers[0].setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

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

	stbi_image_free(pixels);
}

void Texture::release(RenderEngine* engine)
{
	engine->device().destroyImage(image_);
	engine->device().destroyImageView(view_);

	memory_->free(engine->device());
}
