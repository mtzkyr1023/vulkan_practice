﻿#include "render_pipeline.h"
#include "../render_engine.h"
#include "../mk_exception.h"

#include <fstream>
#include <iostream>

GameObjectManager RenderPipeline::objectManager_;
ResourceManager RenderPipeline::resourceManager_;

void RenderPipeline::cleanup(RenderEngine* engine) {
	engine->device().destroyRenderPass(renderPass_);
	engine->device().destroyPipeline(pipeline_);
	engine->device().destroyPipelineLayout(pipelineLayout_);
	engine->device().freeCommandBuffers(engine->commandPool(), commandBuffers_);
	for (vk::Semaphore& semaphore : renderCompletedSemaphores_) {
		engine->device().destroySemaphore(semaphore);
	}
}


vk::ShaderModule RenderPipeline::createShaderModule(
	class RenderEngine* engine,
	const std::string& filename,
	const std::string& entryPoint,
	shaderc::CompileOptions options,
	shaderc_shader_kind shaderKind) {
	std::ifstream ifs;
	ifs.open(filename);
	if (ifs.fail()) {
		throw MkException("Could not read Shader File.");
	}

	size_t filesize = 0;
	std::vector<char> buffer;
	ifs.seekg(0, std::ios_base::end);
	filesize = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);

	buffer.resize(filesize);
	ifs.read((char*)buffer.data(), filesize);

	while (buffer.back() == '\0') {
		buffer.pop_back();
	}

	shaderc::Compiler compiler;

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
		buffer.data(),
		buffer.size(),
		shaderKind,
		filename.c_str(),
		entryPoint.c_str(),
		options);

	ifs.close();

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cout << result.GetErrorMessage() << std::endl;
		throw MkException(result.GetErrorMessage().c_str());
	}

	std::vector<uint32_t> shaderData{result.begin(), result.end()};
	vk::ShaderModuleCreateInfo moduleCreateInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(shaderData.size() * sizeof(uint32_t))
		.setCode(shaderData);

	vk::ShaderModule module = engine->device().createShaderModule(moduleCreateInfo);

	return module;
}