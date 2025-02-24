
#include "deferred_pass.h"
#include "../../render_engine.h"
#include "../../defines.h"

void DeferredPass::setupInternal(RenderEngine* engine)
{
	std::array<vk::AttachmentDescription, 6> attachmentDescs;
	std::array<vk::SubpassDependency, 2> subpassDeps;
	std::array<vk::SubpassDescription, 3> subpassDescs;

	// 使用するAttachments定義
	{
		// 最終出力バッファ
		attachmentDescs[0] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// アルベドバッファ
		attachmentDescs[1] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 法線、Depthバッファ
		attachmentDescs[2] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// Roughness、Metalness、Velocityバッファ
		attachmentDescs[3] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
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
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		// 不透明ライティングバッファ
		attachmentDescs[5] = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
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
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

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
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

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
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference()
			.setAttachment(2)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference()
			.setAttachment(3)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::AttachmentReference()
			.setAttachment(4)
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
		};
		std::array<vk::AttachmentReference, 1> colorAttachments =
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
		//std::array<vk::AttachmentReference, 1> inputAttachments =
		//{
		//	vk::AttachmentReference()
		//	.setAttachment(5)
		//	.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		//};
		//std::array<vk::AttachmentReference, 1> colorAttachments =
		//{
		//	vk::AttachmentReference()
		//	.setAttachment(0)
		//	.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		//};
		//vk::AttachmentReference depthStencilAttachment = vk::AttachmentReference()
		//	.setAttachment(4)
		//	.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);

		//subpassDescs[3] = vk::SubpassDescription()
		//	.setColorAttachments(colorAttachments)
		//	.setInputAttachments(inputAttachments)
		//	.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
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

		//subpassDeps[2] = vk::SubpassDependency()
		//	.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
		//	.setSrcSubpass(2)
		//	.setDstSubpass(3)
		//	.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		//	.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		//	.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
		//	.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);
	}

	vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
		.setAttachments(attachmentDescs)
		.setDependencies(subpassDeps)
		.setSubpasses(subpassDescs);

	renderPass_ = engine->device().createRenderPass(renderPassCreateInfo);

	vk::DeviceSize alignment = 0;

	// GBuffer用のVRAM確保
	{
		memories_.resize(2);
	}
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
			.setArrayLayers(5)
			.setMipLevels(1)
			.setFormat(vk::Format::eR16G16B16A16Sfloat)					// 一旦カラーバッファは RGBA16Float のみ
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		memories_[0].allocateForImage(engine->physicalDevice(), engine->device(), imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

		alignment = kScreenWidth * kScreenHeight * sizeof(float) * 2 + (memories_[0].alignment() - 1) & ~(memories_[0].alignment() - 1);
	}

	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
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

	// 最終出力バッファ作成
	{
		auto& image = images_[ETextureType::eResult];
		auto& view = imageViews_[ETextureType::eResult];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eResult));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// アルベドバッファ作成
	{
		auto& image = images_[ETextureType::eAlbedo];
		auto& view = imageViews_[ETextureType::eAlbedo];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eAlbedo));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// 法線深度バッファ作成
	{
		auto& image = images_[ETextureType::eNormalDepth];
		auto& view = imageViews_[ETextureType::eNormalDepth];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eNormalDepth));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// RoughMetalVelocityバッファ作成
	{
		auto& image = images_[ETextureType::eRoughMetalVelocity];
		auto& view = imageViews_[ETextureType::eRoughMetalVelocity];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eRoughMetalVelocity));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setSubresourceRange(
				vk::ImageSubresourceRange()
				.setLevelCount(1)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = engine->device().createImageView(viewCreateInfo);
	}

	// 不透明ライティング結果バッファ作成
	{
		auto& image = images_[ETextureType::eTemporary];
		auto& view = imageViews_[ETextureType::eTemporary];
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		image = engine->device().createImage(imageCreateInfo);

		memories_[0].bind(engine->device(), image, (vk::DeviceSize)(alignment * ETextureType::eTemporary));

		vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
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
			.setExtent(vk::Extent3D(kScreenWidth, kScreenHeight, 1))
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
		std::array<vk::ImageView, 6> attachments =
		{
			imageViews_[ETextureType::eResult],
			imageViews_[ETextureType::eAlbedo],
			imageViews_[ETextureType::eNormalDepth],
			imageViews_[ETextureType::eRoughMetalVelocity],
			imageViews_[ETextureType::eDepth],
			imageViews_[ETextureType::eTemporary],
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
