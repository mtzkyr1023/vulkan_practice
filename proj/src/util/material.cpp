
#include "material.h"

#include "../render_engine.h"
#include "../resource/memory.h"
#include "../resource/texture.h"

#include "../defines.h"




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
	const char* aoFilename,
	bool generateMipmap)
{
	textures_.resize((uint32_t)ETextureType::eNum);

	{
		std::shared_ptr<Texture> tex = std::make_shared<Texture>();
		tex->setupResource2d(engine, albedoFilename);

		textures_[(uint32_t)ETextureType::eAlbedo] = tex;
	}

	{
		std::shared_ptr<Texture> tex = std::make_shared<Texture>();
		tex->setupResource2d(engine, normalFilename);

		textures_[(uint32_t)ETextureType::eNormal] = tex;
	}

	{
		std::shared_ptr<Texture> tex = std::make_shared<Texture>();
		tex->setupResource2d(engine, pbrFilename);

		textures_[(uint32_t)ETextureType::ePBR] = tex;
	}

	{
		std::shared_ptr<Texture> tex = std::make_shared<Texture>();
		tex->setupResource2d(engine, aoFilename);

		textures_[(uint32_t)ETextureType::eAO] = tex;
	}
}

void Material::release(RenderEngine* engine)
{
	for (const auto& ite : textures_)
	{
		ite->release(engine);
	}
}

void Material::writeDescriptorSet(RenderEngine* engine, uint32_t textureIndex, vk::DescriptorSet set)
{
	vk::WriteDescriptorSet write;

	vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
		.setImageView(textures_[textureIndex]->view())
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setSampler(VK_NULL_HANDLE);

	write = vk::WriteDescriptorSet()
		.setImageInfo(imageInfo)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setDstArrayElement(0)
		.setDstBinding(textureIndex)
		.setDstSet(set);

	engine->device().updateDescriptorSets(write, {});


}
