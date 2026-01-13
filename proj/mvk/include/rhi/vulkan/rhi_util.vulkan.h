
#ifndef _MVK_RHI_VULKAN_RHI_UTIL_
#define _MVK_RHI_VULKAN_RHI_UTIL_


#include <iostream>
#include <vector>
#include <unordered_map>


#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "rhi/rhi.h"

#include "util/types.h"
#include "text/hash_text.h"

namespace mvk
{
	namespace rhi
	{
		vk::Format convertResourceFormat(EResourceFormat v);

		EResourceFormat convertResourceFormat(vk::Format v);

		vk::ImageLayout convertImageLayout(EResourceState v);

		vk::BlendOp convertBlendOp(EBlendOp v);

		vk::BlendFactor convertBlendFactor(EBlendFactor v);

		vk::CullModeFlags convertCullMode(ECullingMode v);

		vk::StencilOp convertStencilOp(EStencilOp v);

		vk::CompareOp convertComparionFunc(ECompFunc v);

		vk::PrimitiveTopology convertPrimitiveTopology(EPrimitiveTopology v);

		vk::Filter convertTextureFilter(ETextureFilterMode v);


		constexpr uint32_t align_top(uint32_t alignment, uint32_t value)
		{
			return (((value + alignment + 1) / alignment) * alignment);
		}
	}
}

#endif
