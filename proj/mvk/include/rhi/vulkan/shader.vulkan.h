
#ifndef _MVK_RHI_VULKAN_SHADER_
#define _MVK_RHI_VULKAN_SHADER_


#include "rhi/rhi.h"
#include "rhi/rhi_ref.h"
#include "rhi/rhi_object_garbage_collect.h"

#include "rhi/vulkan/rhi_util.vulkan.h"



namespace mvk
{
	namespace rhi
	{
		class DeviceDep;

		class ShaderDep
		{
		public:
			ShaderDep();
			virtual ~ShaderDep();

			bool initialize(DeviceDep* p_device, EShaderStage stage, const void* shader_binary_ptr, types::u32 shader_binary_size);

			struct InitFileDesc
			{
				const char* shader_file_path = nullptr;

				const char* entry_point_name = nullptr;
			
				EShaderStage stage = EShaderStage::Vertex;

				const char* shader_model_version = nullptr;

				bool option_debug_mode = false;
				bool option_enable_validation = false;
				bool option_enable_optimization = false;
				bool option_matrix_row_major = false;
			};

			bool initialize(DeviceDep* p_device, const InitFileDesc& desc);
			void finalize();

			types::u32 getShaderBinarySize() const;
			const void* getShaderBinaryPtr() const;
			EShaderStage getShaderStageType() const;

		private:
			EShaderStage stage_;
			std::vector<types::u8> data_;
		};

		class ShaderReflectionDep
		{
		public:
			struct CbInfo
			{
				char name[64];

				types::u16 index;
				types::u16 size;
				types::u16 num_member;
			};

			struct CbVariableInfo
			{
				char name[64];
				types::u16 offset_in_cb;
				types::u16 size;
				types::u16 default_value_offset;
			};

			struct InputParamInfo
			{
				text::HashText<32> semantic_name;

				types::u8 semantic_index;
				types::u8 num_component;
			};

			struct ResourceSlotInfo
			{
				ResourceViewName name;
				ERootParameterType type = ERootParameterType::_Max;
				types::u32 bind_point = -1;
			};

			ShaderReflectionDep();
			~ShaderReflectionDep();

			bool initialize(DeviceDep* p_device, const ShaderDep* p_shader);
			void finalize();

			types::u32 numInputParamInfo() const;
			const InputParamInfo* getInputParamInfo(types::u32 index) const;

			types::u32 numCbInfo() const;
			const CbInfo* getCbInfo(types::u32 index) const;
			const CbVariableInfo* getCbVariableInfo(types::u32 index, types::u32 variable_index);

			template<typename T>
			bool getCbDefaultValue(types::u32 variable_index, T& out) const
			{
				if (cb_.size() <= index || cb_[index].num_member <= variable_index)
					return false;
				const auto cb_var_offset = cb_variable_offset_[index] + variable_index;
				if (cb_variable_[cb_var_offset].size != sizeof(T))
					return false;

				const auto* src = &sb_default_value_buffer_[cb_variable_[cb_var_offset].default_value_offset];
				std::memcpy(&out, src, cb_variable_[cb_var_offset].size);
				return true;
			}

			types::u32 numResourceSlotInfo() const;
			const ResourceSlotInfo* getResourceSlotInfo(types::u32 index) const;

			void getComputeThreadGroupSize(types::u32& out_threadgroup_size_x, types::u32& out_threadgroup_size_y, types::u32& out_threadgroup_size_z) const;

		private:
			std::vector<CbInfo> cb_;

			std::vector<types::u32> cb_variable_offset_;

			std::vector<CbVariableInfo> cb_variable_;

			std::vector<types::u8> cb_default_value_buffer_;


			std::vector<InputParamInfo> input_param_;

			std::vector<ResourceSlotInfo> resource_slot_;

			types::u32 threadgroup_size_x_ = 0;
			types::u32 threadgroup_size_y_ = 0;
			types::u32 threadgroup_size_z_ = 0;
		};



		class PipelineResourceViewLayoutDep
		{
		public:
			struct Desc
			{
				const ShaderDep* vs = nullptr;
				const ShaderDep* gs = nullptr;
				const ShaderDep* ps = nullptr;
				const ShaderDep* cs = nullptr;
			};

			struct DescriptorTableBindInfo
			{
				types::s8 vs_cbv_table = -1;
				types::s8 vs_srv_table = -1;
				types::s8 vs_sampler_table = -1;

				types::s8 gs_cbv_table = -1;
				types::s8 gs_srv_table = -1;
				types::s8 gs_sampler_table = -1;

				types::s8 ps_cbv_table = -1;
				types::s8 ps_srv_table = -1;
				types::s8 ps_sampler_table = -1;

				types::s8 cs_cbv_table = -1;
				types::s8 cs_srv_table = -1;
				types::s8 cs_sampler_table = -1;
				types::s8 cs_uav_table = -1;
			};


			struct ShaderStageSlot
			{
				ERootParameterType type = ERootParameterType::_Max;
				types::s16 slot = -1;
			};

			struct Slot
			{
				ShaderStageSlot vs_stage = {};
				ShaderStageSlot gs_stage = {};
				ShaderStageSlot ps_stage = {};
				ShaderStageSlot cs_stage = {};
			};

			PipelineResourceViewLayoutDep();
			~PipelineResourceViewLayoutDep();

			bool initialize(DeviceDep* p_device, const Desc& desc);
			void finalize();

			const DescriptorTableBindInfo& getResourceTableInfo() const
			{
				return resource_table_;
			}

		private:
			vk::UniqueDescriptorSetLayout layout_;

			ShaderReflectionDep vs_reflection_;

			std::unordered_map<ResourceViewName, Slot> slot_map_;

			DescriptorTableBindInfo resource_table_;
		};
	}
}

#endif
