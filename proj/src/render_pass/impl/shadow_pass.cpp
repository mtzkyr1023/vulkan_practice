
#include "shadow_pass.h"
#include "../../render_engine.h"
#include "../../defines.h"


void ShadowPass::setupInternal(RenderEngine* engine)
{
	std::array<vk::AttachmentDescription, 5> attachmentDescs;
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

		// X方向ブラー画像
		attachmentDescs[1] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// Y方向ブラー画像
		attachmentDescs[2] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 最終出力バッファ
		attachmentDescs[3] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 深度バッファ
		attachmentDescs[4] = vk::AttachmentDescription()
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
			.setAttachment(4)
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

	vk::DeviceSize alignment = 0;

	// VRAM確保
	{
		memories_.resize(2);
	}
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(5)
			.setMipLevels(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		memories_[0].allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		alignment = (vk::DeviceSize)(kShadowMapWidth * kShadowMapHeight * sizeof(float) * 2) + (memories_[0].alignment() - 1) & ~(memories_[0].alignment() - 1);
		alignment = memories_[0].size() / 4;
	}

	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(engine->swapchainImageCount())				// Depthバッファは前フレームの情報を使う拡張性を鑑みてフレームバッファの数
			.setMipLevels(1)
			.setFormat(vk::Format::eD32SfloatS8Uint)					// 
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		memories_[1].allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
	}

	images_.resize(ETextureType::eNum);
	imageViews_.resize(ETextureType::eNum);


	// 生シャドウマップ作成
	{
		auto& image = images_[ETextureType::eRaw];
		auto& view = imageViews_[ETextureType::eRaw];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eRaw));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// BlurXバッファ作成
	{
		auto& image = images_[ETextureType::eBlurX];
		auto& view = imageViews_[ETextureType::eBlurX];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eBlurX));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// BlurYバッファ作成
	{
		auto& image = images_[ETextureType::eBlurY];
		auto& view = imageViews_[ETextureType::eBlurY];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eBlurY));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// リザルトバッファ作成
	{
		auto& image = images_[ETextureType::eResult];
		auto& view = imageViews_[ETextureType::eResult];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eResult));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR32G32Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// 深度ステンシルバッファ作成
	{
		auto& image = images_[ETextureType::eDepth];
		auto& view = imageViews_[ETextureType::eDepth];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kShadowMapWidth, kShadowMapHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[1].bind(engine->device(), image, 0);

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eD32SfloatS8Uint)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eDepth))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	{
		std::array<vk::ImageView, 5> attachments =
		{
			imageViews_[ETextureType::eRaw],
			imageViews_[ETextureType::eBlurX],
			imageViews_[ETextureType::eBlurY],
			imageViews_[ETextureType::eResult],
			imageViews_[ETextureType::eDepth],
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

