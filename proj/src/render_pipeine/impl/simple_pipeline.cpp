#include "simple_pipeline.h"
#include "../../render_engine.h"
#include "../../render_pass/render_pass.h"
#include "../../defines.h"
#include "../../gameobject/camera.h"
#include "../../util/timer.h"
#include "../../render_pass/impl/deferred_pass.h"

#undef MemoryBarrier

SimplePipeline::SimplePipeline() :
	mappedViewProjMemory_(nullptr),
	camera_(Transform(), glm::pi<float>() * 0.5f, (float)kScreenWidth / (float)kScreenHeight, 0.1f, 1000.0f)
{
}

SimplePipeline::~SimplePipeline() {

}

void SimplePipeline::initialize(RenderEngine* engine, RenderPass* pass) {
	{
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings({ binding });

		descriptorLayouts_[ESubpassType::eDepthPrePass].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
	}
	{
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings({ binding });

		descriptorLayouts_[ESubpassType::eGBuffer].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
	}

	{
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings({ binding });

		descriptorLayouts_[ESubpassType::eComposition].push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
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
			.setSetLayouts(descriptorLayouts_[ESubpassType::eDepthPrePass]);

		pipelineLayout_[ESubpassType::eDepthPrePass] = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

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
			.setDepthBoundsTestEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setMaxDepthBounds(1.0f)
			.setMinDepthBounds(0.0f)
			.setStencilTestEnable(false);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/prepass.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/prepass.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_[ESubpassType::eDepthPrePass])
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(pass->renderPass())
			.setPDepthStencilState(&pipelineDepthStencilState)
			.setSubpass(0);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eDepthPrePass] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	{
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
			.setDepthBoundsTestEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
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
			.setSubpass(1);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eGBuffer] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	{
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
			.setDepthBoundsTestEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
			.setDepthTestEnable(true)
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
			.setSubpass(2);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_[ESubpassType::eComposition] = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	commandBuffers_ = engine->allocateCommandBuffer(engine->swapchainImageCount());

	for (uint32_t i = 0; i < (uint32_t)engine->images().size(); i++) {
		renderCompletedSemaphores_.push_back(engine->device().createSemaphore(vk::SemaphoreCreateInfo()));
	}

	ubMemory_.allocateForBuffer(
		engine->physicalDevice(),
		engine->device(),
		vk::BufferCreateInfo()
		.setSize(sizeof(glm::mat4) * 4 * engine->swapchainImageCount())
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer),
		vk::MemoryPropertyFlagBits::eHostVisible);
	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(sizeof(glm::mat4) * 4)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

		viewProjBuffer_.push_back(engine->device().createBuffer(bufferCreateInfo));
	}

	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		ubMemory_.bind(engine->device(), viewProjBuffer_[i], sizeof(glm::mat4) * 4 * i);	
	}

	mappedViewProjMemory_ = ubMemory_.map(engine->device(), 0, sizeof(glm::mat4) * 4 * engine->swapchainImageCount());

	uint32_t descIndex = 0;
	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[ESubpassType::eDepthPrePass]);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			sets_.push_back(engine->device().allocateDescriptorSets(allocInfo)[0]);
		}

		std::vector<vk::WriteDescriptorSet> writes;

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(viewProjBuffer_[i])
				.setOffset(0)
				.setRange(sizeof(glm::mat4) * 4);

			writes.push_back(vk::WriteDescriptorSet()
				.setBufferInfo(bufferInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[descIndex]));
			descIndex++;
		}

		engine->device().updateDescriptorSets(writes, {});
	}

	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[ESubpassType::eGBuffer]);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			sets_.push_back(engine->device().allocateDescriptorSets(allocInfo)[0]);
		}

		std::vector<vk::WriteDescriptorSet> writes;

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(viewProjBuffer_[i])
				.setOffset(0)
				.setRange(sizeof(glm::mat4) * 4);

			writes.push_back(vk::WriteDescriptorSet()
				.setBufferInfo(bufferInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[descIndex]));
			descIndex++;
		}

		engine->device().updateDescriptorSets(writes, {});
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

		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(descriptorLayouts_[ESubpassType::eComposition]);

		sets_.push_back(engine->device().allocateDescriptorSets(allocInfo)[0]);

		std::vector<vk::WriteDescriptorSet> writes;

		vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(pass->imageView((uint32_t)DeferredPass::eAlbedo))
			.setSampler(sampler_);

		writes.push_back(vk::WriteDescriptorSet()
			.setImageInfo(imageInfo)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(0)
			.setDstSet(sets_[descIndex]));

		descIndex++;

		engine->device().updateDescriptorSets(writes, {});
	}

	mesh_.loadMesh(engine, "models/sponza/gltf/", "sponza.gltf");
}

void SimplePipeline::cleanup(RenderEngine* engine) {
	RenderPipeline::cleanup(engine);

	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		engine->device().destroyBuffer(viewProjBuffer_[i]);
	}

	engine->device().destroySampler(sampler_);
	ubMemory_.free(engine->device());

	mesh_.release(engine);

	engine->device().freeDescriptorSets(engine->descriptorPool(), sets_);
	for (uint32_t i = 0; i < ESubpassType::eNum; i++)
	{
		for (const auto& layout : descriptorLayouts_[i])
		{
			engine->device().destroyDescriptorSetLayout(layout);
		}
	}
}

void SimplePipeline::render(RenderEngine* engine, RenderPass* pass, uint32_t currentImageIndex) {
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);

	vk::ClearValue clearValues[DeferredPass::ETextureType::eStencil] = {
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
		vk::ClearDepthStencilValue(1.0f, 0),
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
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
		barriers[0].setImage(pass->image(DeferredPass::eResult));
		barriers[0].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[0].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(colorSubresourceRange);

		barriers[1].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[1].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[1].setImage(pass->image(DeferredPass::eAlbedo));
		barriers[1].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[1].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[1].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[1].setSubresourceRange(colorSubresourceRange);

		barriers[2].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[2].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[2].setImage(pass->image(DeferredPass::eNormalDepth));
		barriers[2].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[2].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[2].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[2].setSubresourceRange(colorSubresourceRange);

		barriers[3].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[3].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[3].setImage(pass->image(DeferredPass::eRoughMetalVelocity));
		barriers[3].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[3].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[3].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[3].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[3].setSubresourceRange(colorSubresourceRange);

		barriers[4].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[4].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[4].setImage(pass->image(DeferredPass::eTemporary));
		barriers[4].setOldLayout(vk::ImageLayout::eUndefined);
		barriers[4].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[4].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[4].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[4].setSubresourceRange(colorSubresourceRange);

		barriers[5].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[5].setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		barriers[5].setImage(pass->image(DeferredPass::eDepth));
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

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eDepthPrePass]);

	cb.bindVertexBuffers(0, { mesh_.vertexBuffer() }, { 0 });
	cb.bindIndexBuffer(mesh_.indexBuffer(), 0, vk::IndexType::eUint32);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_[ESubpassType::eDepthPrePass],
		0,
		{ sets_[currentImageIndex] },
		{});

	cb.drawIndexed(mesh_.allIndexCount(), 1, 0, 0, 0);

	cb.nextSubpass(vk::SubpassContents::eInline);

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eGBuffer]);

	cb.drawIndexed(mesh_.allIndexCount(), 1, 0, 0, 0);

	cb.nextSubpass(vk::SubpassContents::eInline);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_[ESubpassType::eComposition],
		0,
		{ sets_[6] },
		{});
	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_[ESubpassType::eComposition]);

	cb.draw(3, 1, 0, 0);

	//cb.nextSubpass(vk::SubpassContents::eInline);

	cb.endRenderPass();
}

void SimplePipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{
	struct ViewProj
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 world;
	};
	
	ViewProj vp;

	float deltaTime = Timer::instance().deltaTime();

	if (GetAsyncKeyState('W'))
	{
		camera_.transform().position() += camera_.transform().forward() * deltaTime;
	}
	if (GetAsyncKeyState('S'))
	{
		camera_.transform().position() -= camera_.transform().forward() * deltaTime;
	}
	if (GetAsyncKeyState('A'))
	{
		camera_.transform().position() -= camera_.transform().right() * deltaTime;
	}
	if (GetAsyncKeyState('D'))
	{
		camera_.transform().position() += camera_.transform().right() * deltaTime;
	}
	if (GetAsyncKeyState('E'))
	{
		camera_.transform().position() += camera_.transform().up() * deltaTime;
	}
	if (GetAsyncKeyState('Q'))
	{
		camera_.transform().position() -= camera_.transform().up() * deltaTime;
	}

	//if (GetAsyncKeyState(VK_LEFT))
	//{
	//	camera_.transform().rotation() *= glm::rotate(glm::identity<glm::quat>(), deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
	//}

	//if (GetAsyncKeyState(VK_RIGHT))
	//{
	//	camera_.transform().rotation() *= glm::rotate(glm::identity<glm::quat>(), -deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
	//}

	camera_.update(Timer::instance().deltaTime());

	static float rot = 0.0f;

	vp.world =
		glm::mat4(glm::rotate(glm::identity<glm::quat>(), rot, glm::vec3(0.0f, 1.0f, 0.0f))) *
		glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.25f, 0.25f, 0.25f));
	vp.view = camera_.viewMatrix();
	vp.proj = camera_.projMatrix();

	memcpy_s(
		&mappedViewProjMemory_[sizeof(glm::mat4) * 4 * currentImageIndex],
		sizeof(ViewProj),
		&vp,
		sizeof(ViewProj));
}