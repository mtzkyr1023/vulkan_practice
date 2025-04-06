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


	enum class ETextureType
	{
		eAlbedo = 0,
		eNormal,
		ePBR,

		eNum,
	};

	void loadImage(
		class RenderEngine* engine,
		const char* albedoFilename,
		const char* normalFilename,
		const char* pbrFilename,
		bool generateMipmap = false);
	void release(class RenderEngine* engine);

	const vk::Image& image(uint32_t index) { return images_[index]; }
	const vk::ImageView& imageViews(uint32_t index) { return imageViews_[index]; }

	void addDrawInfo(uint32_t indexOffset, uint32_t indexCount)
	{
		drawInfos_.emplace_back(indexOffset, indexCount);
	}
	const DrawInfo& drawInfo(uint32_t index) { return drawInfos_[index]; }

	uint32_t drawInfoCount() { return (uint32_t)drawInfos_.size(); }

	uint32_t indexOffset(uint32_t index) { return drawInfos_[index].indexOffset; }
	uint32_t indexCount(uint32_t index) { return drawInfos_[index].indexCount; }

	EMaterialType materialType() { return materialType_; }

protected:
	std::shared_ptr<class Memory> memory_;

	vk::Image images_[(uint32_t)ETextureType::eNum];
	vk::ImageView imageViews_[(uint32_t)ETextureType::eNum];

	std::vector<DrawInfo> drawInfos_;
	
	EMaterialType materialType_;
};

#endif