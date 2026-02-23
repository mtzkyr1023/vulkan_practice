
#ifndef _MVK_RHI_VULKAN_DEVICE_
#define _MVK_RHI_VULKAN_DEVICE_


#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "platform/win/window.win.h"

#include "rhi/rhi.h"
#include "rhi/rhi_object_garbage_collect.h"

#include "rhi/vulkan/rhi_util.vulkan.h"

namespace mvk
{
	namespace rhi
	{


		class DeviceDep : public IDevice
		{
		public:
			DeviceDep();
			~DeviceDep();

		};

	}
}



#endif

