#include "simple_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../defines.h"
#include "../../gameobject/camera.h"
#include "../../gameobject/scene.h"
#include "../../util/timer.h"
#include "../../util/material.h"
#include "../../util/mesh.h"
#include "../../util/input.h"
#include "../../render_pass/impl/deferred_pass.h"
#include "../../imgui/backends/imgui_impl_vulkan.h"

#include "../../resource/texture.h"
#include "../../resource/buffer.h"

#undef MemoryBarrier

SimplePipeline::SimplePipeline()
{

}

SimplePipeline::~SimplePipeline() {

}

void SimplePipeline::initialize(
	RenderEngine* engine,
	RenderPass* pass,
	const std::vector<class Texture*>& textures,
	const std::vector<class Buffer*>& buffers,
	const std::vector<class Mesh*>& meshes) {

	RenderPipeline::initialize(engine, pass, textures, buffers, meshes);

	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eGBuffer].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 4> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(2)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(3)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eGBuffer].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampler)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eGBuffer].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 4> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(2)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(3)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eComposition].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 3> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(2)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eComposition].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 2> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eComposition].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampler)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eComposition].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eComposition].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eSkybox].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eSkybox].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampler)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[ESubpassType::eSkybox].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		viewport_ = vk::Viewport()
			.setWidth((float)kScreenWidth)
			.setHeight((float)kScreenHeight)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(kScreenWidth, kScreenHeight))
			.setOffset(vk::Offset2D(0, 0));

		vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setScissorCount(1)
			.setPScissors(&scissor)
			.setViewportCount(1)
			.setPViewports(&viewport_);

		std::array<vk::VertexInputAttributeDescription, 4> inputAttributes =
		{
			vk::VertexInputAttributeDescription()
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(0),
			vk::VertexInputAttributeDescription()
			.setLocation(1)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(sizeof(glm::vec4)),
			vk::VertexInputAttributeDescription()
			.setLocation(2)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(sizeof(glm::vec4) + sizeof(glm::vec3)),
			vk::VertexInputAttributeDescription()
			.setLocation(3)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(glm::vec3)),
		};

		std::array<vk::VertexInputBindingDescription, 1> bindAttributes =
		{
			vk::VertexInputBindingDescription()
			.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)
			.setStride(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)),
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
			.setVertexAttributeDescriptions(inputAttributes)
			.setVertexBindingDescriptions(bindAttributes);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasEnable(vk::False)
			.setDepthBiasSlopeFactor(0.0f)
			.setDepthClampEnable(vk::False)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setLineWidth(1.0f)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setRasterizerDiscardEnable(vk::False);

		vk::PipelineMultisampleStateCreateInfo multiSampleState = vk::PipelineMultisampleStateCreateInfo()
			.setAlphaToCoverageEnable(vk::False)
			.setAlphaToOneEnable(vk::False)
			.setMinSampleShading(0.0f)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(vk::False);

		std::array<vk::PipelineColorBlendAttachmentState, 3> colorBlendAttachmentState = 
		{
			vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False),
			vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False),
			vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(vk::False)
			.setAttachments(colorBlendAttachmentState);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[ESubpassType::eGBuffer]);

		pipelineLayout_[ESubpassType::eGBuffer] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

		vk::StencilOpState backState = vk::StencilOpState()
			.setCompareMask(255)
			.setCompareOp(vk::CompareOp::eAlways)
			.setDepthFailOp(vk::StencilOp::eKeep)
			.setFailOp(vk::StencilOp::eKeep)
			.setReference(0)
			.setWriteMask(255);

		vk::StencilOpState frontState = vk::StencilOpState()
			.setCompareMask(255)
			.setCompareOp(vk::CompareOp::eAlways)
			.setDepthFailOp(vk::StencilOp::eKeep)
			.setFailOp(vk::StencilOp::eKeep)
			.setReference(0)
			.setWriteMask(255);

		vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilState = vk::PipelineDepthStencilStateCreateInfo()
			.setBack(backState)
			.setFront(frontState)
			.setDepthBoundsTestEnable(false)
			.setDepthCompareOp(vk::CompareOp::eGreater)
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setMaxDepthBounds(1.0f)
			.setMinDepthBounds(0.0f)
			.setStencilTestEnable(false);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/gbuffer.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/gbuffer.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_[ESubpassType::eGBuffer])
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(pass->renderPass())
			.setPDepthStencilState(&pipelineDepthStencilState)
			.setSubpass(0);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eGBuffer] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	{
		viewport_ = vk::Viewport()
			.setWidth((float)kScreenWidth)
			.setHeight((float)kScreenHeight)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(kScreenWidth, kScreenHeight))
			.setOffset(vk::Offset2D(0, 0));

		vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setScissorCount(1)
			.setPScissors(&scissor)
			.setViewportCount(1)
			.setPViewports(&viewport_);

		std::array<vk::VertexInputAttributeDescription, 0> inputAttributes =
		{
		};

		std::array<vk::VertexInputBindingDescription, 0> bindAttributes =
		{
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
			.setVertexAttributeDescriptions(inputAttributes)
			.setVertexBindingDescriptions(bindAttributes);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasEnable(vk::False)
			.setDepthBiasSlopeFactor(0.0f)
			.setDepthClampEnable(vk::False)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setLineWidth(1.0f)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setRasterizerDiscardEnable(vk::False);

		vk::PipelineMultisampleStateCreateInfo multiSampleState = vk::PipelineMultisampleStateCreateInfo()
			.setAlphaToCoverageEnable(vk::False)
			.setAlphaToOneEnable(vk::False)
			.setMinSampleShading(0.0f)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(vk::False);

		std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachmentState =
		{
			vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(vk::False)
			.setAttachments(colorBlendAttachmentState);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[ESubpassType::eComposition]);

		pipelineLayout_[ESubpassType::eComposition] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

		vk::StencilOpState backState = vk::StencilOpState()
			.setCompareMask(255)
			.setCompareOp(vk::CompareOp::eAlways)
			.setDepthFailOp(vk::StencilOp::eKeep)
			.setFailOp(vk::StencilOp::eKeep)
			.setReference(0)
			.setWriteMask(255);

		vk::StencilOpState frontState = vk::StencilOpState()
			.setCompareMask(255)
			.setCompareOp(vk::CompareOp::eAlways)
			.setDepthFailOp(vk::StencilOp::eKeep)
			.setFailOp(vk::StencilOp::eKeep)
			.setReference(0)
			.setWriteMask(255);

		vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilState = vk::PipelineDepthStencilStateCreateInfo()
			.setBack(backState)
			.setFront(frontState)
			.setDepthBoundsTestEnable(false)
			.setDepthCompareOp(vk::CompareOp::eGreater)
			.setDepthTestEnable(false)
			.setDepthWriteEnable(true)
			.setMaxDepthBounds(1.0f)
			.setMinDepthBounds(0.0f)
			.setStencilTestEnable(false);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/composition.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/composition.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_[ESubpassType::eComposition])
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(pass->renderPass())
			.setPDepthStencilState(&pipelineDepthStencilState)
			.setSubpass(1);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eComposition] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}
	{
		viewport_ = vk::Viewport()
			.setWidth((float)kScreenWidth)
			.setHeight((float)kScreenHeight)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(kScreenWidth, kScreenHeight))
			.setOffset(vk::Offset2D(0, 0));

		vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setScissorCount(1)
			.setPScissors(&scissor)
			.setViewportCount(1)
			.setPViewports(&viewport_);

		std::array<vk::VertexInputAttributeDescription, 4> inputAttributes =
		{
			vk::VertexInputAttributeDescription()
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(0),
			vk::VertexInputAttributeDescription()
			.setLocation(1)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(sizeof(glm::vec4)),
			vk::VertexInputAttributeDescription()
			.setLocation(2)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(sizeof(glm::vec4) + sizeof(glm::vec3)),
			vk::VertexInputAttributeDescription()
			.setLocation(3)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(glm::vec3)),
		};

		std::array<vk::VertexInputBindingDescription, 1> bindAttributes =
		{
			vk::VertexInputBindingDescription()
			.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)
			.setStride(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)),
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
			.setVertexAttributeDescriptions(inputAttributes)
			.setVertexBindingDescriptions(bindAttributes);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasEnable(vk::False)
			.setDepthBiasSlopeFactor(0.0f)
			.setDepthClampEnable(vk::False)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setLineWidth(1.0f)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setRasterizerDiscardEnable(vk::False);

		vk::PipelineMultisampleStateCreateInfo multiSampleState = vk::PipelineMultisampleStateCreateInfo()
			.setAlphaToCoverageEnable(vk::False)
			.setAlphaToOneEnable(vk::False)
			.setMinSampleShading(0.0f)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(vk::False);

		std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachmentState =
		{
			vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(vk::False)
			.setAttachments(colorBlendAttachmentState);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[ESubpassType::eSkybox]);

		pipelineLayout_[ESubpassType::eSkybox] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

		vk::StencilOpState backState = vk::StencilOpState()
			.setCompareMask(255)
			.setCompareOp(vk::CompareOp::eAlways)
			.setDepthFailOp(vk::StencilOp::eKeep)
			.setFailOp(vk::StencilOp::eKeep)
			.setReference(0)
			.setWriteMask(255);

		vk::StencilOpState frontState = vk::StencilOpState()
			.setCompareMask(255)
			.setCompareOp(vk::CompareOp::eAlways)
			.setDepthFailOp(vk::StencilOp::eKeep)
			.setFailOp(vk::StencilOp::eKeep)
			.setReference(0)
			.setWriteMask(255);

		vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilState = vk::PipelineDepthStencilStateCreateInfo()
			.setBack(backState)
			.setFront(frontState)
			.setDepthBoundsTestEnable(false)
			.setDepthCompareOp(vk::CompareOp::eGreaterOrEqual)
			.setDepthTestEnable(true)
			.setDepthWriteEnable(false)
			.setMaxDepthBounds(1.0f)
			.setMinDepthBounds(0.0f)
			.setStencilTestEnable(false);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/skybox.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/skybox.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_[ESubpassType::eSkybox])
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(pass->renderPass())
			.setPDepthStencilState(&pipelineDepthStencilState)
			.setSubpass(2);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eSkybox] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	commandBuffers_ = engine->allocateCommandBuffer(engine->swapchainImageCount());

	for (uint32_t i = 0; i < (uint32_t)engine->images().size(); i++) {
		renderCompletedSemaphores_.push_back(engine->device().createSemaphore(vk::SemaphoreCreateInfo()));
	}

	{
		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(true)
			.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
			.setCompareEnable(false)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMagFilter(vk::Filter::eLinear)
			.setMaxAnisotropy(16.0f)
			.setMinFilter(vk::Filter::eLinear)
			.setMaxLod(FLT_MAX)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear);

		wrapSampler_ = engine->device().createSampler(samplerCreateInfo);
	}

	{
		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setAnisotropyEnable(true)
			.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
			.setCompareEnable(false)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMagFilter(vk::Filter::eLinear)
			.setMaxAnisotropy(16.0f)
			.setMinFilter(vk::Filter::eLinear)
			.setMaxLod(FLT_MAX)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear);

		clampSampler_ = engine->device().createSampler(samplerCreateInfo);
	}


	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setDescriptorSetCount(5)
			.setSetLayouts(descriptorLayouts_[ESubpassType::eGBuffer]);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			for (const auto& ite : meshes)
			{
				for (uint32_t j = 0; j < ite->materialCount(); j++)
				{
					sets_[ESubpassType::eGBuffer].push_back(engine->device().allocateDescriptorSets(allocInfo));
				}
			}
		}

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			{
				const auto& bg = meshes[EMeshType::eBackground];
				for (uint32_t j = 0; j < bg->materialCount(); j++)
				{
					vk::WriteDescriptorSet write;

					vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
						.setBuffer(buffers[EBufferType::eCameraViewProj]->buffer(i))
						.setOffset(0)
						.setRange(sizeof(glm::mat4) * 4);

					write = vk::WriteDescriptorSet()
						.setBufferInfo(bufferInfo)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eUniformBuffer)
						.setDstArrayElement(0)
						.setDstBinding(0)
						.setDstSet(sets_[ESubpassType::eGBuffer][i * bg->materialCount() + j][0]);

					engine->device().updateDescriptorSets(write, {});

					for (uint32_t k = 0; k < (uint32_t)Material::ETextureType::eNum; k++)
					{
						bg->material(j)->writeDescriptorSet(
							engine,
							k,
							sets_[ESubpassType::eGBuffer][i * bg->materialCount() + j][1]);
					}

					vk::DescriptorImageInfo samplerInfo = vk::DescriptorImageInfo()
						.setImageView(VK_NULL_HANDLE)
						.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						.setSampler(wrapSampler_);

					write = vk::WriteDescriptorSet()
						.setImageInfo(samplerInfo)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eSampler)
						.setDstArrayElement(0)
						.setDstBinding(0)
						.setDstSet(sets_[ESubpassType::eGBuffer][i * bg->materialCount() + j][2]);

					engine->device().updateDescriptorSets(write, {});
				}
			}
		}
	}

	
	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[ESubpassType::eComposition]);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			sets_[ESubpassType::eComposition].push_back(engine->device().allocateDescriptorSets(allocInfo));
		}

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(textures[ETextureType::eAlbedo]->view())
					.setSampler(clampSampler_);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eInputAttachment)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eComposition][i][0]);

				engine->device().updateDescriptorSets({ write }, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(textures[ETextureType::eNormalDepth]->view())
					.setSampler(clampSampler_);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eInputAttachment)
					.setDstArrayElement(0)
					.setDstBinding(1)
					.setDstSet(sets_[ESubpassType::eComposition][i][0]);

				engine->device().updateDescriptorSets({ write }, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(textures[ETextureType::eRoughMetalVelocity]->view())
					.setSampler(clampSampler_);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eInputAttachment)
					.setDstArrayElement(0)
					.setDstBinding(2)
					.setDstSet(sets_[ESubpassType::eComposition][i][0]);

				engine->device().updateDescriptorSets({ write }, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(textures[ETextureType::eDepthStencil]->view())
					.setSampler(clampSampler_);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eInputAttachment)
					.setDstArrayElement(0)
					.setDstBinding(3)
					.setDstSet(sets_[ESubpassType::eComposition][i][0]);

				engine->device().updateDescriptorSets({ write }, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
					.setBuffer(buffers[EBufferType::eCameraInv]->buffer(i))
					.setOffset(0)
					.setRange(sizeof(glm::mat4) * 4);

				write = vk::WriteDescriptorSet()
					.setBufferInfo(bufferInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eComposition][i][1]);

				engine->device().updateDescriptorSets(write, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
					.setBuffer(buffers[EBufferType::eSceneInfo]->buffer(i))
					.setOffset(0)
					.setRange(sizeof(glm::mat4) * 4);

				write = vk::WriteDescriptorSet()
					.setBufferInfo(bufferInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setDstArrayElement(0)
					.setDstBinding(1)
					.setDstSet(sets_[ESubpassType::eComposition][i][1]);

				engine->device().updateDescriptorSets(write, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
					.setBuffer(buffers[EBufferType::eShadowViewProj]->buffer(i))
					.setOffset(0)
					.setRange(sizeof(glm::mat4) * 4);

				write = vk::WriteDescriptorSet()
					.setBufferInfo(bufferInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setDstArrayElement(0)
					.setDstBinding(2)
					.setDstSet(sets_[ESubpassType::eComposition][i][1]);

				engine->device().updateDescriptorSets(write, {});
			}
			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageView(textures[ETextureType::eShadowMap]->view())
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setSampler(VK_NULL_HANDLE);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eSampledImage)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eComposition][i][2]);

				engine->device().updateDescriptorSets(write, {});
			}
			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageView(textures[ETextureType::eCubeMap]->view())
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setSampler(VK_NULL_HANDLE);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eSampledImage)
					.setDstArrayElement(0)
					.setDstBinding(1)
					.setDstSet(sets_[ESubpassType::eComposition][i][2]);

				engine->device().updateDescriptorSets(write, {});
			}
			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo samplerInfo = vk::DescriptorImageInfo()
					.setImageView(VK_NULL_HANDLE)
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setSampler(clampSampler_);

				write = vk::WriteDescriptorSet()
					.setImageInfo(samplerInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eSampler)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eComposition][i][3]);

				engine->device().updateDescriptorSets(write, {});
			}
			{
				vk::WriteDescriptorSet write;

				vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
					.setBuffer(buffers[EBufferType::eShCoeff]->buffer(0))
					.setOffset(0)
					.setRange(sizeof(float) * 27);

				write = vk::WriteDescriptorSet()
					.setBufferInfo(bufferInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eStorageBuffer)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eComposition][i][4]);

				engine->device().updateDescriptorSets(write, {});
			}
		}
	}

	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setDescriptorSetCount(5)
			.setSetLayouts(descriptorLayouts_[ESubpassType::eSkybox]);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			sets_[ESubpassType::eSkybox].push_back(engine->device().allocateDescriptorSets(allocInfo));
		}

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			{
				vk::WriteDescriptorSet write;

				vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
					.setBuffer(buffers[EBufferType::eSkyboxInfo]->buffer(i))
					.setOffset(0)
					.setRange(sizeof(glm::mat4) * 4);

				write = vk::WriteDescriptorSet()
					.setBufferInfo(bufferInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eSkybox][i][0]);

				engine->device().updateDescriptorSets({ write }, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(textures[ETextureType::eCubeMap]->view())
					.setSampler(VK_NULL_HANDLE);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eSampledImage)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eSkybox][i][1]);

				engine->device().updateDescriptorSets({ write }, {});
			}

			{
				vk::WriteDescriptorSet write;

				vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(VK_NULL_HANDLE)
					.setSampler(clampSampler_);

				write = vk::WriteDescriptorSet()
					.setImageInfo(imageInfo)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eSampler)
					.setDstArrayElement(0)
					.setDstBinding(0)
					.setDstSet(sets_[ESubpassType::eSkybox][i][2]);

				engine->device().updateDescriptorSets({ write }, {});
			}
		}
	}
}

void SimplePipeline::cleanup(RenderEngine* engine) {
	RenderPipeline::cleanup(engine);

	engine->device().destroySampler(wrapSampler_);
	engine->device().destroySampler(clampSampler_);
}

void SimplePipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex) {
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);

	vk::ClearValue clearValues[DeferredPass::ETextureType::eNum] = {
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearDepthStencilValue(0.0f, 0),
	};

	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		vk::ImageSubresourceRange depthSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 6> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[0].setImage(textures_[ETextureType::eResult]->image());
		barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[0].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[1].setImage(textures_[ETextureType::eAlbedo]->image());
		barriers[1].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[1].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[2].setImage(textures_[ETextureType::eNormalDepth]->image());
		barriers[2].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[2].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		barriers[3].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[3].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[3].setImage(textures_[ETextureType::eRoughMetalVelocity]->image());
		barriers[3].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[3].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[3].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[3].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[3].setSubresourceRange(colorSubresourceRange);

		barriers[4].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[4].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[4].setImage(textures_[ETextureType::eDeferredResult]->image());
		barriers[4].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[4].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[4].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[4].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[4].setSubresourceRange(colorSubresourceRange);

		barriers[5].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[5].setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		barriers[5].setImage(textures_[ETextureType::eDepthStencil]->image());
		barriers[5].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[5].setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		barriers[5].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[5].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[5].setSubresourceRange(depthSubresourceRange);

		cb.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::DependencyFlagBits::eDeviceGroup,
			nullptr,
			nullptr,
			barriers);
	}

	vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setClearValues(clearValues)
		.setFramebuffer(pass->framebuffer())
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(kScreenWidth, kScreenHeight)))
		.setRenderPass(pass->renderPass());

	cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eGBuffer]);

	cb.bindVertexBuffers(0, { meshes_[EMeshType::eBackground]->vertexBuffer()->buffer(0) }, {0});
	cb.bindIndexBuffer(meshes_[EMeshType::eBackground]->indexBuffer()->buffer(0), 0, vk::IndexType::eUint32);

	for (uint32_t i = 0; i < meshes_[EMeshType::eBackground]->materialCount(); i++)
	{
		//if (mesh_.material(i)->isTransparent()) continue;

		cb.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			pipelineLayout_[ESubpassType::eGBuffer],
			0,
			sets_[ESubpassType::eGBuffer][currentImageIndex * meshes_[EMeshType::eBackground]->materialCount() + i],
			{});

		for (uint32_t j = 0; j < meshes_[EMeshType::eBackground]->material(i)->drawInfoCount(); j++)
		{
			cb.drawIndexed(meshes_[EMeshType::eBackground]->material(i)->indexCount(j), 1, meshes_[EMeshType::eBackground]->material(i)->indexOffset(j), 0, 0);
		}
	}

	cb.nextSubpass(vk::SubpassContents::eInline);

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eComposition]);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_[ESubpassType::eComposition],
		0,
		sets_[ESubpassType::eComposition][currentImageIndex],
		{});

	cb.draw(3, 1, 0, 0);

	cb.nextSubpass(vk::SubpassContents::eInline);


	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eSkybox]);


	cb.bindVertexBuffers(0, { meshes_[EMeshType::eSphere]->vertexBuffer()->buffer(0) }, { 0 });
	cb.bindIndexBuffer(meshes_[EMeshType::eSphere]->indexBuffer()->buffer(0), 0, vk::IndexType::eUint32);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_[ESubpassType::eSkybox],
		0,
		sets_[ESubpassType::eSkybox][currentImageIndex],
		{});

	cb.drawIndexed(meshes_[EMeshType::eSphere]->allIndexCount(), 1, 0, 0, 0);

	cb.nextSubpass(vk::SubpassContents::eInline);

	cb.endRenderPass();
}

void SimplePipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{
}