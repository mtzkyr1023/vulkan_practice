#include "fb_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../render_pass/impl/deferred_pass.h"
#include "../../defines.h"

#include "../../resource/texture.h"

FbPipeline::FbPipeline()
{

}

FbPipeline::~FbPipeline()
{

}

void FbPipeline::initialize(
	RenderEngine* engine,
	RenderPass* pass,
	const std::vector<Texture*>& textures,
	const std::vector<Buffer*>& buffers,
	const std::vector<Mesh*>& meshes)
{
	{
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

			descriptorLayouts_[0].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
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

			descriptorLayouts_[0].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		viewport_ = vk::Viewport()
			.setWidth((float)kFramebufferWidth)
			.setHeight((float)kFramebufferHeight)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(kFramebufferWidth, kFramebufferHeight))
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

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False);

		vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
			.setAttachmentCount(1)
			.setLogicOpEnable(vk::False)
			.setPAttachments(&colorBlendAttachmentState);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[0]);

		pipelineLayout_[0] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

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
			.setDepthCompareOp(vk::CompareOp::eAlways)
			.setDepthTestEnable(false)
			.setDepthWriteEnable(false)
			.setMaxDepthBounds(1.0f)
			.setMinDepthBounds(0.0f)
			.setStencilTestEnable(false);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/fb.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/fb.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_[0])
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(engine->renderPass())
			.setPDepthStencilState(&pipelineDepthStencilState)
			.setSubpass(0);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[0] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
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
			.setMaxLod(0)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear);

		sampler_ = engine->device().createSampler(samplerCreateInfo);
	}

	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[0]);

		sets_[0].push_back(engine->device().allocateDescriptorSets(allocInfo));

		std::vector<vk::WriteDescriptorSet> writes;

		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(textures[ETextureType::eSrc]->view())
				.setSampler(VK_NULL_HANDLE);

			writes.push_back(vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[0][0][0]));
		}

		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(VK_NULL_HANDLE)
				.setSampler(sampler_);

			writes.push_back(vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampler)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[0][0][1]));
		}

		engine->device().updateDescriptorSets(writes, {});
	}

	textures_ = textures;
	buffers_ = buffers;
	meshes_ = meshes;

}

void FbPipeline::cleanup(RenderEngine* engine)
{
	RenderPipeline::cleanup(engine);

	engine->device().destroySampler(sampler_);
}


void FbPipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex)
{
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);

	vk::ClearValue clearValues[1] = {
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
	};

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[0]);
	
	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_[0],
		0,
		sets_[0][0],
		{});

	cb.draw(3, 1, 0, 0);
}

void FbPipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{

}
