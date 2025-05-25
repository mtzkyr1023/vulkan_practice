
#include "shadow_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../render_pass/impl/shadow_pass.h"
#include "../../defines.h"
#include "../../util/material.h"
#include "../../util/input.h"
#include "../../util/timer.h"
#include "../../resource/texture.h"
#include "../../resource/buffer.h"

ShadowPipeline::ShadowPipeline()
{

}

ShadowPipeline::~ShadowPipeline()
{

}

void ShadowPipeline::initialize(
	RenderEngine* engine,
	RenderPass* pass,
	const std::vector<class Texture*>& textures,
	const std::vector<class Buffer*>& buffers,
	const std::vector<class Mesh*>& meshes)
{
	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 2> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);
			descriptorLayouts_[ESubpassType::eRaw].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
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
			descriptorLayouts_[ESubpassType::eRaw].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
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
			descriptorLayouts_[ESubpassType::eRaw].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		viewport_ = vk::Viewport()
			.setWidth((float)kShadowMapWidth)
			.setHeight((float)kShadowMapHeight)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(kShadowMapWidth, kShadowMapHeight))
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
			.setSetLayouts(descriptorLayouts_[ESubpassType::eRaw]);

		pipelineLayout_[ESubpassType::eRaw] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

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
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setMaxDepthBounds(1.0f)
			.setMinDepthBounds(0.0f)
			.setStencilTestEnable(false);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/shadow.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/shadow.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_[ESubpassType::eRaw])
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(pass->renderPass())
			.setPDepthStencilState(&pipelineDepthStencilState)
			.setSubpass(0);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eRaw] = result.value;

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
			.setMaxLod(FLT_MAX)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear);

		sampler_ = engine->device().createSampler(samplerCreateInfo);
	}

	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setDescriptorSetCount(5)
			.setSetLayouts(descriptorLayouts_[ESubpassType::eRaw]);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			for (const auto& ite : meshes)
			{
				for (uint32_t j = 0; j < ite->materialCount(); j++)
				{
					sets_[ESubpassType::eRaw].push_back(engine->device().allocateDescriptorSets(allocInfo));
				}
			}
		}

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			for (const auto& ite : meshes)
			{
				for (uint32_t j = 0; j < ite->materialCount(); j++)
				{
					vk::WriteDescriptorSet write;

					{
						vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
							.setBuffer(buffers[EBufferType::eShadowViewProj]->buffer(i))
							.setOffset(0)
							.setRange(sizeof(glm::mat4) * 4);

						write = vk::WriteDescriptorSet()
							.setBufferInfo(bufferInfo)
							.setDescriptorCount(1)
							.setDescriptorType(vk::DescriptorType::eUniformBuffer)
							.setDstArrayElement(0)
							.setDstBinding(0)
							.setDstSet(sets_[ESubpassType::eRaw][i * ite->materialCount() + j][0]);

						engine->device().updateDescriptorSets(write, {});
					}

					{
						vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
							.setBuffer(buffers[EBufferType::eCameraViewPoj]->buffer(i))
							.setOffset(0)
							.setRange(sizeof(glm::mat4) * 4);

						write = vk::WriteDescriptorSet()
							.setBufferInfo(bufferInfo)
							.setDescriptorCount(1)
							.setDescriptorType(vk::DescriptorType::eUniformBuffer)
							.setDstArrayElement(0)
							.setDstBinding(1)
							.setDstSet(sets_[ESubpassType::eRaw][i * ite->materialCount() + j][0]);

						engine->device().updateDescriptorSets(write, {});
					}

					ite->material(j)->writeDescriptorSet(
						engine,
						0,
						sets_[ESubpassType::eRaw][i * ite->materialCount() + j][1]);

					vk::DescriptorImageInfo samplerInfo = vk::DescriptorImageInfo()
						.setImageView(VK_NULL_HANDLE)
						.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						.setSampler(sampler_);

					write = vk::WriteDescriptorSet()
						.setImageInfo(samplerInfo)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eSampler)
						.setDstArrayElement(0)
						.setDstBinding(0)
						.setDstSet(sets_[ESubpassType::eRaw][i * ite->materialCount() + j][2]);

					engine->device().updateDescriptorSets(write, {});
				}
			}
		}
	}

	buffers_ = buffers;
	textures_ = textures;
	meshes_ = meshes;
}

void ShadowPipeline::cleanup(RenderEngine* engine)
{
	RenderPipeline::cleanup(engine);

	engine->device().destroySampler(sampler_);
}

void ShadowPipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex)
{
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);

	vk::ClearValue clearValues[ShadowPass::ETextureType::eNum] = {
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearDepthStencilValue(1.0f, 0),
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

		std::array<vk::ImageMemoryBarrier, 2> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[0].setImage(textures_[ShadowPass::eRaw]->image());
		barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[0].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		barriers[1].setImage(textures_[ShadowPass::eDepth]->image());
		barriers[1].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[1].setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(depthSubresourceRange);

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
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(kShadowMapWidth, kShadowMapHeight)))
		.setRenderPass(pass->renderPass());

	cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eRaw]);

	for (const auto& ite : meshes_)
	{
		cb.bindVertexBuffers(0, { ite->vertexBuffer()->buffer(0) }, {0});
		cb.bindIndexBuffer(ite->indexBuffer()->buffer(0), 0, vk::IndexType::eUint32);

		for (uint32_t i = 0; i < ite->materialCount(); i++)
		{
			//if (mesh_.material(i)->isTransparent()) continue;
			cb.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				pipelineLayout_[ESubpassType::eRaw],
				0,
				sets_[ESubpassType::eRaw][currentImageIndex * ite->materialCount() + i],
				{});

			for (uint32_t j = 0; j < ite->material(i)->drawInfoCount(); j++)
			{
				cb.drawIndexed(ite->material(i)->indexCount(j), 1, ite->material(i)->indexOffset(j), 0, 0);
			}
		}
	}

	cb.endRenderPass();
}

void ShadowPipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{

}
