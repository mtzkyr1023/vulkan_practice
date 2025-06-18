#include "shadow_blur_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../render_pass/impl/deferred_pass.h"
#include "../../defines.h"

#include "../../resource/texture.h"
#include "../../resource/buffer.h"

ShadowBlurPipeline::ShadowBlurPipeline()
{

}

ShadowBlurPipeline::~ShadowBlurPipeline()
{

}

void ShadowBlurPipeline::initialize(
	RenderEngine* engine,
	RenderPass* pass,
	const std::vector<Texture*>& textures,
	const std::vector<Buffer*>& buffers,
	const std::vector<Mesh*>& meshes)
{
	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 2> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eBlurX].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eBlurX].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 2> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
				vk::DescriptorSetLayoutBinding()
				.setBinding(1)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eBlurY].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eBlurY].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[EPassType::eBlurX]);

		pipelineLayout_[EPassType::eBlurX] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(1);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eCompute);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/shadow_blur_x.comp", "main", shaderc::CompileOptions(), shaderc_compute_shader));

		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(0)
			.setLayout(pipelineLayout_[EPassType::eBlurX])
			.setStage(shaderStages[0]);

		vk::ResultValue<vk::Pipeline> result = engine->device().createComputePipeline(
			engine->pipelineCache(),
			computePipelineCreateInfo);

		pipeline_[EPassType::eBlurX] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
	}

	{
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[EPassType::eBlurY]);

		pipelineLayout_[EPassType::eBlurY] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(1);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eCompute);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/shadow_blur_y.comp", "main", shaderc::CompileOptions(), shaderc_compute_shader));

		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(0)
			.setLayout(pipelineLayout_[EPassType::eBlurY])
			.setStage(shaderStages[0]);

		vk::ResultValue<vk::Pipeline> result = engine->device().createComputePipeline(
			engine->pipelineCache(),
			computePipelineCreateInfo);

		pipeline_[EPassType::eBlurY] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
	}

	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[EPassType::eBlurX]);

		sets_[EPassType::eBlurX].push_back(engine->device().allocateDescriptorSets(allocInfo));

		vk::WriteDescriptorSet writes;

		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eGeneral)
				.setImageView(textures[ETextureType::eSrc]->view())
				.setSampler(VK_NULL_HANDLE);

			writes = vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eBlurX][0][0]);

			engine->device().updateDescriptorSets(writes, {});
		}
		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eGeneral)
				.setImageView(textures[ETextureType::eDstX]->view())
				.setSampler(VK_NULL_HANDLE);

			writes = vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setDstArrayElement(0)
				.setDstBinding(1)
				.setDstSet(sets_[EPassType::eBlurX][0][0]);

			engine->device().updateDescriptorSets(writes, {});
		}

		{
			vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(buffers[EBufferType::eGaussWeights]->buffer(0))
				.setOffset(0)
				.setRange(sizeof(float) * kBlurSize * 4);

			writes = vk::WriteDescriptorSet()
				.setBufferInfo(bufferInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eBlurX][0][1]);

			engine->device().updateDescriptorSets(writes, {});
		}
	}

	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[EPassType::eBlurY]);

		sets_[EPassType::eBlurY].push_back(engine->device().allocateDescriptorSets(allocInfo));

		vk::WriteDescriptorSet writes;

		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eGeneral)
				.setImageView(textures[ETextureType::eDstX]->view())
				.setSampler(VK_NULL_HANDLE);

			writes = vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eBlurY][0][0]);

			engine->device().updateDescriptorSets(writes, {});
		}
		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eGeneral)
				.setImageView(textures[ETextureType::eDstY]->view())
				.setSampler(VK_NULL_HANDLE);

			writes = vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setDstArrayElement(0)
				.setDstBinding(1)
				.setDstSet(sets_[EPassType::eBlurY][0][0]);

			engine->device().updateDescriptorSets(writes, {});
		}

		{
			vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(buffers[EBufferType::eGaussWeights]->buffer(0))
				.setOffset(0)
				.setRange(sizeof(float) * kBlurSize * 4);

			writes = vk::WriteDescriptorSet()
				.setBufferInfo(bufferInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eBlurY][0][1]);

			engine->device().updateDescriptorSets(writes, {});
		}
	}

	textures_ = textures;
	buffers_ = buffers;
	meshes_ = meshes;

}

void ShadowBlurPipeline::cleanup(RenderEngine* engine)
{
	RenderPipeline::cleanup(engine);
}


void ShadowBlurPipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex)
{
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);
	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 3> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(textures_[ETextureType::eSrc]->image());
		barriers[0].setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::eGeneral);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eShaderWrite);
		barriers[1].setImage(textures_[ETextureType::eDstX]->image());
		barriers[1].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[1].setNewLayout(vk::ImageLayout::eGeneral);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eShaderWrite);
		barriers[2].setImage(textures_[ETextureType::eDstY]->image());
		barriers[2].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[2].setNewLayout(vk::ImageLayout::eGeneral);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		cb.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::DependencyFlagBits::eDeviceGroup,
			nullptr,
			nullptr,
			barriers);
	}

	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_[EPassType::eBlurX]);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		pipelineLayout_[EPassType::eBlurX],
		0,
		sets_[EPassType::eBlurX][0],
		{});

	cb.dispatch(kShadowMapWidth / 256, kShadowMapHeight, 1);

	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 1> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eShaderWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(textures_[ETextureType::eDstX]->image());
		barriers[0].setOldLayout(vk::ImageLayout::eGeneral);
		barriers[0].setNewLayout(vk::ImageLayout::eGeneral);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		cb.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::DependencyFlagBits::eDeviceGroup,
			nullptr,
			nullptr,
			barriers);
	}


	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_[EPassType::eBlurY]);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		pipelineLayout_[EPassType::eBlurY],
		0,
		sets_[EPassType::eBlurY][0],
		{});

	cb.dispatch(kShadowMapHeight / 256, kShadowMapWidth, 1);


	{
		vk::ImageSubresourceRange colorSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::array<vk::ImageMemoryBarrier, 3> barriers;
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[0].setImage(textures_[ETextureType::eSrc]->image());
		barriers[0].setOldLayout(vk::ImageLayout::eGeneral);
		barriers[0].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[1].setImage(textures_[ETextureType::eDstX]->image());
		barriers[1].setOldLayout(vk::ImageLayout::eGeneral);
		barriers[1].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eShaderWrite);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		barriers[2].setImage(textures_[ETextureType::eDstY]->image());
		barriers[2].setOldLayout(vk::ImageLayout::eGeneral);
		barriers[2].setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		cb.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::DependencyFlagBits::eDeviceGroup,
			nullptr,
			nullptr,
			barriers);
	}
}

void ShadowBlurPipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{

}
