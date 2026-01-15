
#ifndef _MVK_RHI_VULKAN_COMMAND_LIST_
#define _MVK_RHI_VULKAN_COMMAND_LIST_

#include <vulkan/vulkan.hpp>

#include "util/types.h"

#include "rhi/rhi.h"
#include "rhi/rhi_object_garbage_collect.h"

#include "rhi/vulkan/rhi_util.vulkan.h"

namespace mvk
{
	namespace rhi
	{
		class CommandListBaseDep : public RhiObjectBase
		{
		public:
			struct Desc
			{

			};

		public:
			CommandListBaseDep() = default;
			virtual ~CommandListBaseDep() = default;

			bool initialize(DeviceDep* p_device, const Desc& desc);

		public:
			void begin();
			void end();


		};
	}
}

#endif
