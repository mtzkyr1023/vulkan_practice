
#include "shadow_pass.h"
#include "../../render_engine.h"
#include "../../defines.h"
#include "../../resource/texture.h"


void ShadowPass::setupInternal(RenderEngine* engine, const std::vector<Texture*>& resources)
{
	std::array<vk::AttachmentDescription, 2> attachmentDescs;
	std::array<vk::SubpassDependency, 0> subpassDeps;
	std::array<vk::SubpassDescription, 1> subpassDescs;

	{
		// 生のシャドウマップ
		attachmentDescs[0] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 深度バッファ
		attachmentDescs[1] = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	}

	// ShadowMap Subpass の定義
	{
		std::array<vk::AttachmentReference, 0> inputAttachments;
		std::array<vk::AttachmentReference, 1> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};
		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		subpassDescs[0] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPDepthStencilAttachment(&depthStencilAttachment)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}

	{
		//subpassDeps[0] = vk::SubpassDependency()
		//	.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
		//	.setSrcSubpass(0)
		//	.setDstSubpass(1)
		//	.setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
		//	.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		//	.setSrcAccessMask(vk::AccessFlagBits::eNone)
		//	.setDstAccessMask(vk::AccessFlagBits::eNone);
	}

	vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
		.setAttachments(attachmentDescs)
		.setDependencies(subpassDeps)
		.setSubpasses(subpassDescs);

	renderPass_ = engine->device().createRenderPass(renderPassCreateInfo);

	{
		std::array<vk::ImageView, 2> attachments =
		{
			resources[ETextureType::eRaw]->view(),
			resources[ETextureType::eDepth]->view(),
		};
		vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
			.setWidth(kShadowMapWidth)
			.setHeight(kShadowMapHeight)
			.setRenderPass(renderPass_)
			.setLayers(1)
			.setAttachments(attachments);

		framebuffer_ = engine->device().createFramebuffer(framebufferCreateInfo);
	}
}

