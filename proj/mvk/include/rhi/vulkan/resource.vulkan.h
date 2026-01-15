
#ifndef _MVK_RHI_VULKAN_RESOURCE_
#define _MVK_RHI_VULKAN_RESOURCE_

#include <vulkan/vulkan.hpp>

#include "rhi/rhi.h"
#include "rhi/rhi_object_garbage_collect.h"

#include "util/types.h"
#include "text/hash_text.h"

namespace mvk
{
	namespace rhi
	{
		class GraphicsCommandListDep;

		class BufferDep : public RhiObjectBase
		{
		public:
			struct Desc
			{
				types::u32 element_stride = 0;
				types::u32 element_count = 0;
				types::u32 bind_flag = 0;
				EResourceHeapType heap_type = EResourceHeapType::Upload;
				EResourceState initial_state = EResourceState::Common;

				void setupAsConstantBuffer(types::u32 size)
				{
					heap_type = EResourceHeapType::Upload;
					initial_state = EResourceState::General;

					bind_flag = (int)ResourceBindFlag::ConstantBuffer;
					element_stride = size;
					element_count = 1;
				}
			};

			BufferDep();
			~BufferDep();

			bool initialize(DeviceDep* p_device, const Desc& desc);
			void finalize();

			void* map();
			template<typename T>
			T* mapAs()
			{
				return (T*)map();
			}
			void unmap();

			bool isValid() const { return buffer_.get() != VK_NULL_HANDLE; }

			const Desc& getDesc() { return desc_; }

		public:
			types::u32 getBufferSize() const { return allocated_byte_size_; }
			types::u32 getElementStride() const { return desc_.element_stride; }
			types::u32 getElementCount() const { return desc_.element_count; }

		private:
			Desc desc_ = {};
			types::u32 allocated_byte_size_ = 0;

			void* map_ptr_ = nullptr;
			vk::UniqueBuffer buffer_;
		};



		struct TextureUploadSubresourceInfo
		{
			types::s32 array_index = {};
			types::s32 mip_index = {};
			types::s32 slize_index = {};

			types::s32 width = {};
			types::s32 height = {};
			EResourceFormat format = {};
			types::s32 row_pitch = {};
			types::s32 slice_pitch = {};
			types::u8* pixels = {};
		};

		struct TextureSubresourceLayoutInfo
		{
			types::u64 byte_offset = 0;

			EResourceFormat format = EResourceFormat::Format_UNKNOWN;

			types::u32 width = 0;
			types::u32 height = 0;
			types::u32 depth = 0;
			types::u32 row_pitch = 0;
		};


		class TextureDep : public RhiObjectBase
		{
		public:
			struct Desc
			{
				EResourceFormat format = EResourceFormat::Format_UNKNOWN;
				types::u32 width = 1;
				types::u32 height = 1;
				types::u32 depth = 1;
				types::u32 mip_count = 1;
				types::u32 sample_count = 1;
				types::u32 array_size = 1;
				
				ETextureType type = ETextureType::Texture2D;

				types::u32 bind_flag = 0;
				EResourceHeapType heap_type = EResourceHeapType::Default;
				EResourceState initial_state = EResourceState::Common;

				bool is_default_clear_value = true;

				struct DepthStencil
				{
					float clear_value = 1.0f;
				} depth_stencil = {};
				struct RenderTarget
				{
					std::array<float, 4> clear_value = { 0.0f, 0.0f, 0.0f, 0.0f };
				} render_target = {};
			};

		public:
			static void initializeBase(Desc& out_desc, ETextureType dimension_type, EResourceFormat format, bool rtv, bool uav)
			{
				out_desc.type = dimension_type;
				out_desc.format = format;

				out_desc.bind_flag = ResourceBindFlag::ShaderResource;
				if (rtv)
					out_desc.bind_flag = ResourceBindFlag::RenderTarget;
				if (uav)
					out_desc.bind_flag = ResourceBindFlag::UnorderedAccess;
			}

			static void initializeAsTexture2D(Desc& out_desc, EResourceFormat format, types::u32 w, types::u32 h, bool rtv, bool uav)
			{
				initializeBase(out_desc, ETextureType::Texture2D, format, rtv, uav);
			
				out_desc.width = w;
				out_desc.height = h;
				out_desc.depth = 1;
				out_desc.array_size = 1;
				out_desc.mip_count = 1;
			}

			static void initializeAsCubemap(Desc& out_desc, EResourceFormat format, u32 w, u32 h, bool rtv, bool uav)
			{
				initializeBase(out_desc, ETextureType::TextureCube, format, rtv, uav);

				out_desc.width = w;
				out_desc.height = h;
				out_desc.depth = 1;
				out_desc.array_size = 6;
				out_desc.mip_count = 1;
			}

			TextureDep();
			~TextureDep();

			bool initialize(DeviceDep* p_device, const Desc& desc);
			void finalize();

			void* map();
			void unmap();

			bool isValid() const { return image_.get() != VK_NULL_HANDLE; }

			const Desc& getDesc() const { return desc_; }
			
			int numSubresource() const;

			void getSubresourceLayoutInfo(TextureSubresourceLayoutInfo* out_layout_array, types::u64& out_toral_byte_size) const;
			void copyTextureRegion(GraphicsCommandListDep* p_command_list, int subresource_index, const BufferDep* p_src_buffer, const TextureSubresourceLayoutInfo& src_layout);

			vk::Image getVulkanImage() const;

		public:
			types::u32 getBufferSize() const { return allocated_byte_size_; }

			types::u32 getWidth(int mip_level = 0) const { return (mip_level == 0) || (mip_level < desc_.mip_count) ? std::max<>(1U, desc_.width >> mip_level) : 0; }
			types::u32 getHeight(int mip_level = 0) const { return (mip_level == 0) || (mip_level < desc_.mip_count) ? std::max<>(1U, desc_.height >> mip_level) : 0; }
			types::u32 getDepth(int mip_level = 0) const { return (mip_level == 0) || (mip_level < desc_.mip_count) ? std::max<>(1U, desc_.depth >> mip_level) : 0; }

			types::u32 getMipCount() const { return desc_.mip_count; }

			types::u32 getSampleCount() const { return desc_.sample_count; }

			EResourceFormat getFormat() const { return desc_.format; }

			types::u32 getBindFlag() const { return desc_.bind_flag; }

			ETextureType getType() const { return desc_.type; }

		private:
			Desc desc_ = {};
			types::u32 allocated_byte_size_ = 0;

			void* map_ptr_ = nullptr;

			vk::UniqueImage image_;
		};
	}
}

#endif
