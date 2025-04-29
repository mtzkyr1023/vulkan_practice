
#include "shadow_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../render_pass/impl/shadow_pass.h"
#include "../../defines.h"
#include "../../util/material.h"
#include "../../util/input.h"
#include "../../util/timer.h"
#include "../../resource/texture.h"

ShadowPipeline::ShadowPipeline()
{

}

ShadowPipeline::~ShadowPipeline()
{

}

void ShadowPipeline::initialize(RenderEngine* engine, RenderPass* pass, const std::vector<std::shared_ptr<Texture>>& textures)
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

	ubMemory_.allocateForBuffer(
		engine->physicalDevice(),
		engine->device(),
		vk::BufferCreateInfo()
		.setSize(sizeof(glm::mat4) * 4 * engine->swapchainImageCount() * 3)
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer),
		vk::MemoryPropertyFlagBits::eHostVisible);
	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(sizeof(glm::mat4) * 4)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

		viewProjBuffer_.push_back(engine->device().createBuffer(bufferCreateInfo));
		cameraBuffer_.push_back(engine->device().createBuffer(bufferCreateInfo));
	}

	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		ubMemory_.bind(engine->device(), viewProjBuffer_[i], sizeof(glm::mat4) * 4 * i + sizeof(glm::mat4) * 4 * engine->swapchainImageCount() * 0);
		ubMemory_.bind(engine->device(), cameraBuffer_[i], sizeof(glm::mat4) * 4 * i + sizeof(glm::mat4) * 4 * engine->swapchainImageCount() * 1);
	}

	mappedViewProjMemory_ = ubMemory_.map(engine->device(), 0, sizeof(glm::mat4) * 4 * engine->swapchainImageCount());

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
			for (uint32_t j = 0; j < scene->bgMesh()->mesh().materialCount(); j++)
			{
				sets_[ESubpassType::eRaw].push_back(engine->device().allocateDescriptorSets(allocInfo));
			}
		}

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			for (uint32_t j = 0; j < scene->bgMesh()->mesh().materialCount(); j++)
			{
				vk::WriteDescriptorSet write;

				{
					vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
						.setBuffer(viewProjBuffer_[i])
						.setOffset(0)
						.setRange(sizeof(glm::mat4) * 4);

					write = vk::WriteDescriptorSet()
						.setBufferInfo(bufferInfo)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eUniformBuffer)
						.setDstArrayElement(0)
						.setDstBinding(0)
						.setDstSet(sets_[ESubpassType::eRaw][i * scene->bgMesh()->mesh().materialCount() + j][0]);

					engine->device().updateDescriptorSets(write, {});
				}

				{
					vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
						.setBuffer(cameraBuffer_[i])
						.setOffset(0)
						.setRange(sizeof(glm::mat4) * 4);

					write = vk::WriteDescriptorSet()
						.setBufferInfo(bufferInfo)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eUniformBuffer)
						.setDstArrayElement(0)
						.setDstBinding(1)
						.setDstSet(sets_[ESubpassType::eRaw][i * scene->bgMesh()->mesh().materialCount() + j][0]);

					engine->device().updateDescriptorSets(write, {});
				}

				{
					vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
						.setImageView(scene->bgMesh()->mesh().material(j)->imageViews(0))
						.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						.setSampler(VK_NULL_HANDLE);

					write = vk::WriteDescriptorSet()
						.setImageInfo(imageInfo)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eSampledImage)
						.setDstArrayElement(0)
						.setDstBinding(0)
						.setDstSet(sets_[ESubpassType::eRaw][i * scene->bgMesh()->mesh().materialCount() + j][1]);

					engine->device().updateDescriptorSets(write, {});
				}

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
					.setDstSet(sets_[ESubpassType::eRaw][i * scene->bgMesh()->mesh().materialCount() + j][2]);

				engine->device().updateDescriptorSets(write, {});
			}
		}
	}
}

void ShadowPipeline::cleanup(RenderEngine* engine)
{
	RenderPipeline::cleanup(engine);

	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		engine->device().destroyBuffer(viewProjBuffer_[i]);
	}

	engine->device().destroySampler(sampler_);
	ubMemory_.free(engine->device());
}

void ShadowPipeline::render(RenderEngine* engine, RenderPass* pass, Scene* scene, uint32_t currentImageIndex)
{
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);

	vk::ClearValue clearValues[ShadowPass::ETextureType::eNum] = {
		vk::ClearColorValue(1.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
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

		std::array<vk::ImageMemoryBarrier, 4> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[0].setImage(pass->image(ShadowPass::eResult));
		barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[0].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[1].setImage(pass->image(ShadowPass::eBlurX));
		barriers[1].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[1].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[2].setImage(pass->image(ShadowPass::eBlurY));
		barriers[2].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[2].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		barriers[3].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[3].setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		barriers[3].setImage(pass->image(ShadowPass::eDepth));
		barriers[3].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[3].setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		barriers[3].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[3].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[3].setSubresourceRange(depthSubresourceRange);

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

	cb.bindVertexBuffers(0, { scene->bgMesh()->mesh().vertexBuffer() }, { 0 });
	cb.bindIndexBuffer(scene->bgMesh()->mesh().indexBuffer(), 0, vk::IndexType::eUint32);

	for (uint32_t i = 0; i < scene->bgMesh()->mesh().materialCount(); i++)
	{
		//if (mesh_.material(i)->isTransparent()) continue;
		cb.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			pipelineLayout_[ESubpassType::eRaw],
			0,
			sets_[ESubpassType::eRaw][currentImageIndex * scene->bgMesh()->mesh().materialCount() + i],
			{});

		for (uint32_t j = 0; j < scene->bgMesh()->mesh().material(i)->drawInfoCount(); j++)
		{
			cb.drawIndexed(scene->bgMesh()->mesh().material(i)->indexCount(j), 1, scene->bgMesh()->mesh().material(i)->indexOffset(j), 0, 0);
		}
	}

	cb.endRenderPass();
}

void ShadowPipeline::update(RenderEngine* engine, Scene* scene, uint32_t currentImageIndex)
{

	struct ViewProj
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 world;
		glm::vec4 sceneInfo;
	};

	struct Camera
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 world;
	};

	ViewProj vp;
	Camera camera;

	const float speed = 100.0f;

	float deltaTime = Timer::instance().deltaTime();



	scene->shadowCaster().transform().rotation() *=
		glm::rotate(glm::identity<glm::quat>(), Input::Instance().GetMoveYLeftPushed() * 2.0f * -deltaTime, scene->shadowCaster().transform().right()) *
		glm::rotate(glm::identity<glm::quat>(), Input::Instance().GetMoveXLeftPushed() * 2.0f * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

	scene->shadowCaster().update(Timer::instance().deltaTime());

	static float rot = 0.0f;

	vp.world =
		glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.25f, 0.25f, 0.25f));
	vp.view = scene->shadowCaster().viewMatrix();
	vp.proj = scene->shadowCaster().projMatrix();
	vp.sceneInfo = glm::vec4((float)kShadowMapWidth, (float)kShadowMapHeight, scene->shadowCaster().nearZ(), scene->shadowCaster().farZ());

	camera.view = scene->camera().viewMatrix();
	camera.proj = scene->camera().projMatrix();

	memcpy_s(
		&mappedViewProjMemory_[sizeof(glm::mat4) * 4 * currentImageIndex + sizeof(glm::mat4) * 4 * engine->swapchainImageCount() * 0],
		sizeof(ViewProj),
		&vp,
		sizeof(ViewProj));

	memcpy_s(
		&mappedViewProjMemory_[sizeof(glm::mat4) * 4 * currentImageIndex + sizeof(glm::mat4) * 4 * engine->swapchainImageCount() * 1],
		sizeof(camera),
		&camera,
		sizeof(camera));
}
