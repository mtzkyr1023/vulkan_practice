#include "render_pipeline.h"
#include "render_engine.h"
#include "mk_exception.h"

#include <fstream>


void RenderPipeline::cleanup(RenderEngine* engine) {
	engine->device_.destroyRenderPass(renderPass_);
	engine->device_.destroyPipeline(pipeline_);
}


vk::ShaderModule RenderPipeline::createShaderModule(RenderEngine* engine, const std::string& filename) {
	std::vector<uint8_t> bin;
	std::ifstream ifs;
	ifs.open(filename);
	if (ifs.fail()) {
		throw MkException("Could not read Shader File.");
	}
}