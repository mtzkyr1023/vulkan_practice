
#define STB_IMAGE_IMPLEMENTATION


#include "material.h"

#include "../render_engine.h"
#include "../render_pipeine/resource.h"

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
	int width, height, bpp;
	albedo = stbi_load(albedoFilename, &width, &height, &bpp, 4);

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

		alignment = width * height * sizeof(float) + (memory_->alignment() - 1) & ~(memory_->alignment() - 1);
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
			.setUsage(vk::ImageUsageFlagBits::eSampled);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, alignment * (vk::DeviceSize)type);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
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
			.setUsage(vk::ImageUsageFlagBits::eSampled);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, alignment * (vk::DeviceSize)type);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
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
			.setUsage(vk::ImageUsageFlagBits::eSampled);

		image = engine->device().createImage(imageCreateInfo);

		memory_->bind(engine->device(), image, alignment * (vk::DeviceSize)type);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	//if (albedo != nullptr)
	//{
	//	uint32_t type = (uint32_t)ETextureType::eAlbedo;
	//	uint8_t* mappedMemory = memory_->map(engine->device(), (vk::DeviceSize)(alignment * type), alignment);

	//	memcpy_s(mappedMemory, alignment, albedo, alignment);

	//	memory_->unmap(engine->device());
	//}

	//if (normal != nullptr)
	//{
	//	uint32_t type = (uint32_t)ETextureType::eNormal;
	//	uint8_t* mappedMemory = memory_->map(engine->device(), (vk::DeviceSize)(alignment * type), alignment);

	//	memcpy_s(mappedMemory, alignment, normal, alignment);

	//	memory_->unmap(engine->device());
	//}

	//if (pbr != nullptr)
	//{
	//	uint32_t type = (uint32_t)ETextureType::ePBR;
	//	uint8_t* mappedMemory = memory_->map(engine->device(), (vk::DeviceSize)(alignment * type), alignment);

	//	memcpy_s(mappedMemory, alignment, pbr, alignment);

	//	memory_->unmap(engine->device());
	//}

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