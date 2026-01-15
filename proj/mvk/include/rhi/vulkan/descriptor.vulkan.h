
#ifndef _MVK_RHI_VULKAN_DESCRIPTOR_
#define _MVK_RHI_VULKAN_DESCRIPTOR_

#include <mutex>

#include <vulkan/vulkan.hpp>

#include "rhi/rhi.h"
#include "rhi/rhi_object_garbage_collect.h"

#include "text/hash_text.h"
#include "util/types.h"


namespace mvk
{
	namespace rhi
	{
		class DeviceDep;
		class PersistantDescriptorAllocator;
		class FrameCommandListDescriptorInterface;

		class DescriptorSetDep
		{
		private:
			template<types::u32 SIZE>
			struct Handles
			{
				void reset();
			};

		public:
			DescriptorSetDep() {}
			~DescriptorSetDep() {}

			void reset();
			
		private:
			Handles<k_cbv_table_size> vs_cbv_;
			Handles<k_srv_table_size> vs_srv_;
			Handles<k_sampler_table_size> vs_sampler_;
			Handles<k_cbv_table_size> fs_cbv_;
			Handles<k_srv_table_size> fs_srv_;
			Handles<k_sampler_table_size> fs_sampler_;
		};

		static constexpr auto k_sizeof_DescriptorSetDep = sizeof(DescriptorSetDep);

		class DescriptorHeapWrapper
		{
		public:
			struct Desc
			{

			};

			DescriptorHeapWrapper();
			~DescriptorHeapWrapper();

			bool initialize(DeviceDep* p_device, const Desc& desc);
			void finalize();

			const Desc& getDesc() const { ; }
		
			vk::DescriptorSet getVulkan() { return heap_.get(); }
			const vk::DescriptorSet getVulkan() const { return heap_.get(); }

		private:
			Desc desc_ = {};

			vk::UniqueDescriptorSet heap_;
		};

		struct PersistantDescriptorInfo
		{
			static constexpr types::u32 k_invalid_allocation_index = ~types::u32(0);

			static bool isValid(const PersistantDescriptorInfo& v)
			{
				return k_invalid_allocation_index != v;
			}

		};

		class PersistantDescriptorAllocator
		{
		public:
			struct Desc
			{
				types::u32 allocate_descriptor_count = 50000;
			};

			PersistantDescriptorAllocator();
			~PersistantDescriptorAllocator();

			bool initialize(DeviceDep* p_device, const Desc& desc);
			void finalize();

			PersistantDescriptorInfo allocate();
			void deallocate(const PersistantDescriptorInfo& v);

		private:
			std::mutex mutex_;

			Desc desc_ = {};

			types::u32 num_use_flag_elem_ = 0;
			std::vector<types::u32> use_flag_bit_array_;

			types::u32 last_allocate_index_ = 0;

			types::u32 num_allocated_ = 0;

			types::u32 tail_fraction_bit_mask_ = 0;

			DescriptorHeapWrapper heap_wrapper_ = {};

			PersistantDescriptorInfo default_persistant_descriptor_;

		private:
			static constexpr types::u32 k_num_flag_elem_bit_ = sizeof(decltype(*use_flag_bit_array_.data())) * 8;
		};


		namespace dynamic_descriptor_allocator
		{
			struct RangeHandle
			{
				RangeHandle()
					: data(0)
				{
				}

				bool isValid() const
				{
					return detail.size != 0;
				}

				union
				{
					types::u64 data;
					struct
					{
						types::u32 head;
						types::u32 size;
					} detail;
				};
			};

			class RangeAllocator
			{
			public:
				RangeAllocator();
				~RangeAllocator();

				bool initialize(types::u32 max_size);
				void finalize();

				RangeHandle alloc(types::u32 size);
				void dealloc(const RangeHandle& handle);

				types::u32 maxSize() const;

				types::u32 calcTotalFreeSize() const;

			private:
				class RangeAllocationImpl* impl_ = nullptr;
			};
		}

		using DynamicDescriptorAllocHandle = dynamic_descriptor_allocator::RangeHandle;


		class DynamicDescriptorManager
		{
		public:
			struct Desc
			{
				types::u32 allocate_descriptor_count_ = 50000;
			};

			DynamicDescriptorManager();
			~DynamicDescriptorManager();

			bool initialize(DeviceDep* p_device, const Desc& desc);
			void finalize();

			void readyToNewFrame(types::u32 frame_index);

			DynamicDescriptorAllocHandle allocateDescriptorArray(types::u32 count);
			void deallocate(const DynamicDescriptorAllocHandle& handle, types::u32 frame_index);
			void deallocateDeferred(const DynamicDescriptorAllocHandle& handle, types::u32 frame_index);

			types::u32 getMaxDescriptorCount() const;
			types::u32 getFreeDescriptorCount() const;

			types::u32 getHandleIncrementSize() const
			{

			}
		};
	}
}

#endif
