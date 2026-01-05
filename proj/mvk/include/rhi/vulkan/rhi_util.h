
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
	}
}

#endif
