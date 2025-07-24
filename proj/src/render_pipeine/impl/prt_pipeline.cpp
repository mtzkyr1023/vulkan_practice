#include "prt_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../defines.h"

#include "../../resource/texture.h"
#include "../../resource/buffer.h"

PrtPipeline::PrtPipeline(uint32_t cubemapSize)
	: cubemapSize_(cubemapSize)
{

}

PrtPipeline::~PrtPipeline()
{

}

void PrtPipeline::initialize(
	RenderEngine* engine,
	RenderPass* pass,
	const std::vector<Texture*>& textures,
	const std::vector<Buffer*>& buffers,
	const std::vector<Mesh*>& meshes)
{
	RenderPipeline::initialize(engine, pass, textures, buffers, meshes);
	{
		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eSh].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}

		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eSh].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}


		{
			std::array<vk::DescriptorSetLayoutBinding, 1> binding =
			{
				vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampler)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute),
			};

			vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(binding);

			descriptorLayouts_[EPassType::eSh].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
		}
	}

	{
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descriptorLayouts_[EPassType::eSh]);

		pipelineLayout_[EPassType::eSh] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);


		shaderc::CompileOptions options;
		{
			char buf[128];
			sprintf(buf, "%d", cubemapSize_);
			options.AddMacroDefinition("CUBEMAP_SIZE", buf);
		}
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(1);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eCompute);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/prt.comp", "main", options, shaderc_compute_shader));

		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(0)
			.setLayout(pipelineLayout_[EPassType::eSh])
			.setStage(shaderStages[0]);

		vk::ResultValue<vk::Pipeline> result = engine->device().createComputePipeline(
			engine->pipelineCache(),
			computePipelineCreateInfo);

		pipeline_[EPassType::eSh] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
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
			.setSetLayouts(descriptorLayouts_[EPassType::eSh]);

		sets_[EPassType::eSh].push_back(engine->device().allocateDescriptorSets(allocInfo));

		vk::WriteDescriptorSet writes;

		{
			vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(buffers[EBufferType::eShCoeffs]->buffer(0))
				.setOffset(0)
				.setRange(sizeof(float) * 3 * 9);

			writes = vk::WriteDescriptorSet()
				.setBufferInfo(bufferInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageBuffer)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eSh][0][0]);

			engine->device().updateDescriptorSets(writes, {});
		}
		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(textures[ETextureType::eCubemap]->view())
				.setSampler(VK_NULL_HANDLE);

			writes = vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eSh][0][1]);

			engine->device().updateDescriptorSets(writes, {});
		}
		{
			vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(VK_NULL_HANDLE)
				.setSampler(sampler_);

			writes = vk::WriteDescriptorSet()
				.setImageInfo(imageInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampler)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[EPassType::eSh][0][2]);

			engine->device().updateDescriptorSets(writes, {});
		}
	}
}

void PrtPipeline::cleanup(RenderEngine* engine)
{
	RenderPipeline::cleanup(engine);
	engine->device().destroySampler(sampler_);
}

void PrtPipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex)
{
	std::vector<vk::CommandBuffer> commandBuffers = engine->allocateCommandBuffer(1);

	auto& cb = commandBuffers[0];

	cb.begin(vk::CommandBufferBeginInfo());

	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_[EPassType::eSh]);

	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout_[EPassType::eSh], 0, sets_[EPassType::eSh][0], {});

	cb.dispatch(1, 1, 1);

	cb.end();

	vk::Fence submitFence = engine->device().createFence(vk::FenceCreateInfo());

	vk::SubmitInfo submitInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&cb);

	engine->graphicsQueue().submit(submitInfo, submitFence);

	vk::Fence waitFences[1] = {
		submitFence,
	};

	engine->device().waitForFences(waitFences, vk::True, kTimeOut);

	engine->device().destroyFence(submitFence);
}


void PrtPipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{

}
