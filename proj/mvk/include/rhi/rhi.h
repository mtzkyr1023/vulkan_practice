
#ifndef _MVK_RHI_
#define _MVK_RHI_

#include <iostream>
#include <vector>
#include <cassert>
#include <cstddef>

#include "util/types.h"
#include "text/hash_text.h"

namespace mvk
{
	namespace rhi
	{
		using namespace mvk::types;
		using ResourceViewName = mvk::text::HashText<32>;

		enum class ERootParameterType : u16
		{
			ConstantBuffer = 0,
			ShaderResource,
			UnorderedAccess,
			Sampler,

			_Max,
		};

		static const u32 k_cbv_table_size = 16;
		static const u32 k_src_table_size = 16;
		static const u32 k_uav_table_size = 16;
		static const u32 k_sampler_table_size = 16;

		static constexpr u32 rootParameterTableSize(
			ERootParameterType type)
		{
			const u32 type_size[] =
			{
				k_cbv_table_size,
				k_src_table_size,
				k_uav_table_size,
				k_sampler_table_size,

				0,
			};

			static_assert(std::size(type_size) - 1 == static_cast<size_t>(ERootParameterType::_Max), "");
			return type_size[static_cast<u32>(type)];
		}

		template<typename T0, typename T1>
		static constexpr bool check_bits(T0 v0, T1 v1)
		{
			return 0 != (v0 & v1);
		}

		enum class EResourceFormat : int
		{
			Format_UNKNOWN = 0,
			_Max,
		};

		inline EResourceFormat depthToColorFormat(EResourceFormat format)
		{
			switch (format)
			{
			default:
				return format;
			}
		}

		inline bool isDepthFormat(EResourceFormat format)
		{
			return false;
		}

		enum class EResourceState
		{
			Common = 0,
			General,
			ConstantBuffer,
			VertexBuffer,
			IndexBuffer,
			RenderTarget,
			ShaderRead,
			UnorderedAccess,
			DepthWrite,
			DepthRead,
			IndirectArgument,
			CopyDst,
			CopySrc,
			Present,
		};


		enum class EResourceHeapType
		{
			Default = 0,
			Upload,
			Readback,
		};

		enum class EShaderStage
		{
			Vertex = 0,
			Hull,
			Domain,
			Geometry,
			Fragment,

			Compute,

			ShaderLibrary,

			_Max,
		};


		using ShaderStageMask = u32;
		static constexpr ShaderStageMask getShaderStageMask(EShaderStage stage)
		{
			return 1u << static_cast<u32>(stage);
		}
		static constexpr bool isContainShaderStage(ShaderStageMask mask, EShaderStage stage)
		{
			return 0 != (mask & getShaderStageMask(stage));
		}


		enum class EBlendFactor
		{
			Zero = 0,
			One,
			SrcColor,
			InvSrcColor,
			SrcAlpha,
			InvSrcAlpha,
			DstAlpha,
			InvDstAlpha,
			DstColor,
			InvDstColor,
			SrcAlphaSet,
			BlendFactor,
			InvBlendFactor,
		};

		enum class EBlendOp
		{
			Add = 0,
			Subtract,
			RevSubtract,
			Min,
			Max,
		};

		enum class ECompFunc
		{
			Never = 0,
			Less,
			Equal,
			LessEqual,
			Greater,
			NotEqual,
			GreaterEqual,
			Always,
		};


		enum class EStencilOp
		{
			Keep = 0,
			Zero,
			Replace,
			IncrSat,
			DecrSat,
			Incr,
			Decr,
		};

		enum class EStencilFace
		{
			Front = 0,
			Back,

			_Max,
		};

		enum class EFillMode
		{
			Wireframe,
			Solid,
		};

		enum class ECullingMode
		{
			None,
			Front,
			Back,
		};


		enum class ETextureFilterMode
		{
			Anisotropic,
		};


		enum class ETextureAdressMode
		{
			Reapeat = 0,
			Mirror,
			Clamp,
			Border,
			MirrorOne,
		};


		enum class EPrimitiveTopologyType
		{
			Point = 0,
			Line,
			Triangle,
			Patch,
		};

		enum class EPrimitiveTopology
		{
			Undefined = 0,
			PointList,
			LineList,
			LineStrip,
			TriangleList,
			TriangleStrip,
		};

		struct SampleDesc
		{
			u32 count = 1;
			u32 quality = 0;
		};

		struct RenderTargetBlendState
		{
			bool blend_enable = false;
			EBlendFactor src_color_blend = EBlendFactor::SrcAlpha;
			EBlendFactor dst_color_blend = EBlendFactor::InvSrcAlpha;
			EBlendOp color_op = EBlendOp::Add;
			EBlendFactor src_alpha_blend = EBlendFactor::SrcAlpha;
			EBlendFactor dst_alpha_blend = EBlendFactor::DstAlpha;
			EBlendOp alpha_op = EBlendOp::Add;
			u8 write_mask = static_cast<u8>(~0u);
		};

		struct BlendState
		{
			bool alpha_to_coverage_enable = false;
			bool independent_blend_enable = false;
			RenderTargetBlendState target_blend_state[8] = {};
		};


		struct RasterizerState
		{
			EFillMode fill_moed = EFillMode::Solid;
			ECullingMode cull_mode = ECullingMode::Back;
			int depth_bias = 0;
			u32 force_sample_count = 0;
			float depth_bias_clamp = 0.0f;
			float slope_scale_depth_bias = 0.0f;
			bool depth_clip_enable = true;
			bool multi_sample_enable = false;
			bool antialiased_line_enable = false;
			bool onservative_raster = false;
			bool front_counter_clockwise = false;
		};

		struct DepthStencilOp
		{
			EStencilOp stencil_fail_op = EStencilOp::Keep;
			EStencilOp stencil_depth_fail_op = EStencilOp::Keep;
			EStencilOp stencil_pass_op = EStencilOp::Keep;
			ECompFunc stencil_func = ECompFunc::Always;
		};

		struct DepthStencilState
		{
			ECompFunc depth_func = ECompFunc::Always;
			DepthStencilOp front_face = {};
			DepthStencilOp back_face = {};
			bool depth_enable = false;
			bool depth_write_enable = true;
			bool stencil_enable = false;
			u8 stencil_read_mask = u8(~0u);
			u8 stencil_write_mask = u8(~0u);
		};

		struct StreamOutputDesc
		{
			u32 num_entries = 0;
		};

		struct InputElement
		{
			const char* semantic_name = {};
			u32 semantic_index = 0;
			EResourceFormat format = EResourceFormat::Format_UNKNOWN;
			u32 stream_output = 0;
			u32 element_offset = 0;
		};

		struct InputLayout
		{
			const InputElement* p_input_elements = nullptr;
			u32 num_elements = 0;
		};

		enum class ETextureType : u32
		{
			Texture1D = 0,
			Texture2D,
			Texture3D,
			TextureCube,
			Texture2DMultisample
		};

		enum class EResourceDimension : u32
		{
			Unknown = 0,
			Texture1D,
			Texture2D,
			Texture3D,
			TextureCube,
			Texture1DArray,
			Texture2DArrat,
			Texture2DMS,
			Texture2DMSArray,
			TextureCubeArray,
			AccelerationStructure,
			Buffer,

			Count,
		};

		struct ResourceBindFlag
		{
			static constexpr u32 None = (1 << 0);
			static constexpr u32 ConstantBuffer = (1 << 1);
			static constexpr u32 VertexBuffer = (1 << 2);
			static constexpr u32 IndexBuffer = (1 << 3);
			static constexpr u32 ShaderResource = (1 << 4);
			static constexpr u32 UnorderedAccess = (1 << 5);
			static constexpr u32 RenderTarget = (1 << 6);
			static constexpr u32 DepthStencil = (1 << 7);
			static constexpr u32 IndirectArg = (1 << 8);
		};


	}
}

#endif
