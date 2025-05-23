﻿
#include "deferred_pass.h"
#include "../../render_engine.h"
#include "../../defines.h"

#include "../../resource/texture.h"

void DeferredPass::setupInternal(RenderEngine* engine, const std::vector<Texture*>& resources)
{
	std::array<vk::AttachmentDescription, 6> attachmentDescs;
	std::array<vk::SubpassDependency, 3> subpassDeps;
	std::array<vk::SubpassDescription, 4> subpassDescs;

	// 使用するAttachments定義
	{
		// 最終出力バッファ
		attachmentDescs[ETextureType::eResult] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// アルベドバッファ
		attachmentDescs[ETextureType::eAlbedo] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 法線、Depthバッファ
		attachmentDescs[ETextureType::eNormalDepth] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// Roughness、Metalness、Velocityバッファ
		attachmentDescs[ETextureType::eRoughMetalVelocity] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// Deferred出力バッファ
		attachmentDescs[ETextureType::eComposition] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 深度バッファ
		attachmentDescs[ETextureType::eDepth] = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	}

	// G-Buffer作成 Subpass の定義
	{
		std::array<vk::AttachmentReference, 0> inputAttachments;
		std::array<vk::AttachmentReference, 3> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(ETextureType::eAlbedo)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(ETextureType::eNormalDepth)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(ETextureType::eRoughMetalVelocity)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};
		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(ETextureType::eDepth)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		subpassDescs[0] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPDepthStencilAttachment(&depthStencilAttachment)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}
	
	// ライティング Subpass の定義
	{
		std::array<vk::AttachmentReference, 4> inputAttachments = 
		{
			vk::AttachmentReference()
			.setAttachment(ETextureType::eAlbedo)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference()
			.setAttachment(ETextureType::eNormalDepth)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference()
			.setAttachment(ETextureType::eRoughMetalVelocity)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference()
			.setAttachment(ETextureType::eDepth)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
		};
		std::array<vk::AttachmentReference, 1> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(ETextureType::eComposition)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};

		subpassDescs[1] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}
	
	// スカイボックス Subpass の定義
	{
		std::array<vk::AttachmentReference, 0> inputAttachments =
		{
		};
		std::array<vk::AttachmentReference, 1> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(ETextureType::eComposition)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};

		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(ETextureType::eDepth)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		subpassDescs[2] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPDepthStencilAttachment(&depthStencilAttachment)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}

	// 透明 Subpass の定義
	{
		std::array<vk::AttachmentReference, 1> inputAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(ETextureType::eComposition)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		};
		std::array<vk::AttachmentReference, 1> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(ETextureType::eResult)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};
		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(ETextureType::eDepth)
			.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);

		subpassDescs[3] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}

	{
		subpassDeps[0] = vk::SubpassDependency()
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcSubpass(0)
			.setDstSubpass(1)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);

		subpassDeps[1] = vk::SubpassDependency()
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcSubpass(1)
			.setDstSubpass(2)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);

		subpassDeps[2] = vk::SubpassDependency()
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcSubpass(2)
			.setDstSubpass(3)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);
	}

	vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
		.setAttachments(attachmentDescs)
		.setDependencies(subpassDeps)
		.setSubpasses(subpassDescs);

	renderPass_ = engine->device().createRenderPass(renderPassCreateInfo);

	{
		std::array<vk::ImageView, 6> attachments =
		{
			resources[ETextureType::eResult]->view(),
			resources[ETextureType::eAlbedo]->view(),
			resources[ETextureType::eNormalDepth]->view(),
			resources[ETextureType::eRoughMetalVelocity]->view(),
			resources[ETextureType::eComposition]->view(),
			resources[ETextureType::eDepth]->view(),
		};
		vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
			.setWidth(kScreenWidth)
			.setHeight(kScreenHeight)
			.setRenderPass(renderPass_)
			.setLayers(1)
			.setAttachments(attachments);

		framebuffer_ = engine->device().createFramebuffer(framebufferCreateInfo);
	}
}
