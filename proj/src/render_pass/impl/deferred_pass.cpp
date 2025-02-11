
#include "deferred_pass.h"
#include "../../render_engine.h"

void DeferredPass::setupInternal(RenderEngine* engine)
{
	std::array<vk::AttachmentDescription, 6> attachmentDescs;
	std::array<vk::SubpassDependency, 3> subpassDeps;
	std::array<vk::SubpassDescription, 4> subpassDescs;

	// 使用するAttachments定義
	{
		// 最終出力バッファ
		attachmentDescs[0] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// アルベドバッファ
		attachmentDescs[1] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 法線、Depthバッファ
		attachmentDescs[2] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// Roughness、Metalness、Velocityバッファ
		attachmentDescs[3] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 深度バッファ
		attachmentDescs[4] = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setInitialLayout(vk::ImageLayout::eDepthAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 不透明ライティングバッファ
		attachmentDescs[5] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	}

	// Depth-Prepass Subpass の定義
	{
		std::array<vk::AttachmentReference, 0> inputAttachments;
		std::array<vk::AttachmentReference, 0> colorAttachments;
		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(4)
			.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);

		subpassDescs[0] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPDepthStencilAttachment(&depthStencilAttachment)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}

	// G-Buffer作成 Subpass の定義
	{
		std::array<vk::AttachmentReference, 0> inputAttachments;
		std::array<vk::AttachmentReference, 3> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(2)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(3)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};
		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(4)
			.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);

		subpassDescs[1] = vk::SubpassDescription()
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
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(2)
			.setLayout(vk::ImageLayout::eAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(3)
			.setLayout(vk::ImageLayout::eAttachmentOptimal),
			vk::AttachmentReference()
			.setAttachment(4)
			.setLayout(vk::ImageLayout::eAttachmentOptimal)
		};
		std::array<vk::AttachmentReference, 4> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(5)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};

		subpassDescs[2] = vk::SubpassDescription()
			.setColorAttachments(colorAttachments)
			.setInputAttachments(inputAttachments)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	}

	// 透明 Subpass の定義
	{
		std::array<vk::AttachmentReference, 4> inputAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(5)
		};
		std::array<vk::AttachmentReference, 4> colorAttachments =
		{
			vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		};
		vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
			.setAttachment(4)
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
			.setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eNone)
			.setDstAccessMask(vk::AccessFlagBits::eNone);

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
		.setDependencies()
		.setSubpasses(subpassDescs);
}
