#ifndef _MATERIAL_H_
#define _MATERIAL_H_


#include "vulkan/vulkan.hpp"

enum class EMaterialType : uint32_t
{
	eOpaque = 0,
	eMask,
	eTransparent,
};

struct DrawInfo
{
	DrawInfo(uint32_t indexOffset, uint32_t indexCount)
	{
		this->indexOffset = indexOffset;
		this->indexCount = indexCount;
	}
	uint32_t indexOffset;
	uint32_t indexCount;
};

class Material
{
public:
	Material(EMaterialType type);
	~Material();


	enum ETextureType
	{
		eAlbedo = 0,
		eNormal,
		ePBR,
		eAO,

		eNum,
	};

	void loadImage(
		class RenderEngine* engine,
		const char* albedoFilename,
		const char* normalFilename,
		const char* pbrFilename,
		const char* aoFilename,
		bool generateMipmap = false);
	void release(class RenderEngine* engine);

	const std::shared_ptr<class Texture>& texture(uint32_t index) { return textures_[index]; }

	void addDrawInfo(uint32_t indexOffset, uint32_t indexCount)
	{
		drawInfos_.emplace_back(indexOffset, indexCount);
	}
	const DrawInfo& drawInfo(uint32_t index) { return drawInfos_[index]; }

	uint32_t drawInfoCount() { return (uint32_t)drawInfos_.size(); }

	uint32_t indexOffset(uint32_t index) { return drawInfos_[index].indexOffset; }
	uint32_t indexCount(uint32_t index) { return drawInfos_[index].indexCount; }

	EMaterialType materialType() { return materialType_; }

	void writeDescriptorSet(class RenderEngine* engine, uint32_t textureIndex, vk::DescriptorSet set);
	
protected:
	std::vector<std::shared_ptr<class Texture>> textures_;
	std::vector<DrawInfo> drawInfos_;
	
	EMaterialType materialType_;
};

#endif