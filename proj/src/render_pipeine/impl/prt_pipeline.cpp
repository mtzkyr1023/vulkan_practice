#include "prt_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../defines.h"

#include "../../resource/texture.h"
#include "../../resource/buffer.h"

PrtPipeline::PrtPipeline()
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
		options.AddMacroDefinition("CUBEMAP_SIZE", "512");
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
}

void PrtPipeline::cleanup(RenderEngine* engine)
{
	RenderPipeline::cleanup(engine);
}

void PrtPipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex)
{

}


void PrtPipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{

}
