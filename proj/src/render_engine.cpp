

#include <iostream>

#include "defines.h"
#include "mk_exception.h"
#include "render_engine.h"


void RenderEngine::initializeContext() {


	std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if defined(_DEBUG)
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
	};

	const vk::ApplicationInfo applicationInfo = vk::ApplicationInfo()
		.setPApplicationName(kAppName)
		.setPEngineName(kAppName)
		.setEngineVersion(0)
#if VK_VERSION_MINOR >= 3
		.setApiVersion(VK_API_VERSION_1_3);
#else
		.setApiVersion(VK_API_VERSION_1_3);
#endif


	const vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo()
		.setFlags(static_cast<vk::InstanceCreateFlagBits>(0))
		.setPApplicationInfo(&applicationInfo)
		.setEnabledExtensionCount((uint32_t)extensions.size())
		.setPEnabledExtensionNames(extensions)
		.setEnabledLayerCount((uint32_t)layers.size())
		.setPEnabledLayerNames(layers);

	std::cout << "creating vulkan instance..." << std::endl;
	instance_ = vk::createInstance(instanceCreateInfo);
	if (!instance_) {
		throw MkException("vulkan instance creation error");
	}
	std::cout << "succeeded createing vulkan instance." << std::endl;

	physicalDevices_ = instance_.enumeratePhysicalDevices();

	for (int i = 0; i < (int)physicalDevices_.size(); i++) {
		vk::PhysicalDevice physicalDevice = physicalDevices_[i];
		vk::PhysicalDeviceProperties prop = physicalDevice.getProperties();
		std::cout << i << ":" << prop.deviceName << std::endl;
	}

	{
		int select = 0;

		physicalDevice_ = physicalDevices_[select];
	}

	{
		graphicsQueueFamilyIndex_ = 0;
		std::vector<vk::QueueFamilyProperties> queueProps = physicalDevice_.getQueueFamilyProperties();
		uint32_t queueCount = (uint32_t)queueProps.size();
		for (uint32_t i = 0; i < queueCount; i++) {
			if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				graphicsQueueFamilyIndex_ = i;
				break;
			}
		}

		float queuePriproties[] = {
			0.0f
		};
		const vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
			.setQueueCount(1)
			.setQueueFamilyIndex(graphicsQueueFamilyIndex_)
			.setPQueuePriorities(queuePriproties);

		vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice_.getFeatures();

		std::vector<const char*> enabledExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined(_DEBUG)
			VK_EXT_DEBUG_MARKER_EXTENSION_NAME
#endif
		};

		const vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(1)
			.setPQueueCreateInfos(&queueCreateInfo)
			.setPEnabledFeatures(&deviceFeatures)
			.setEnabledExtensionCount((uint32_t)enabledExtensions.size())
			.setPEnabledExtensionNames(enabledExtensions)
			.setEnabledLayerCount((uint32_t)layers.size())
			.setPEnabledLayerNames(layers);

		device_ = physicalDevice_.createDevice(deviceCreateInfo);
		graphicsQueue_ = device_.getQueue(graphicsQueueFamilyIndex_, 0);


		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(graphicsQueueFamilyIndex_)
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		commandPool_ = device_.createCommandPool(commandPoolCreateInfo);
	}

#if defined(_DEBUG)
	{
		createDebugreportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");
		destroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");
		debugBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance_, "vkDebugReportMessageEXT");

		VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
		vk::DebugReportFlagsEXT flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning;
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debugCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
		debugCreateInfo.flags = flags.operator VkSubpassDescriptionFlags();

		VkResult err = createDebugreportCallback(instance_, &debugCreateInfo, nullptr, &msgCallback);
		assert(!err);
	}
#endif
}

void RenderEngine::cleanupContext() {
	graphicsQueue_.waitIdle();
	device_.waitIdle();

	device_.destroyPipelineCache(pipelineCache_);

	device_.destroyCommandPool(commandPool_);

	device_.destroy();

	destroyDebugReportCallback(instance_, msgCallback, nullptr);

	instance_.destroy();
}

void RenderEngine::initializeSurface(HWND hwnd, HINSTANCE hinstance) {
	{
		const vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR()
			.setHwnd(hwnd)
			.setHinstance(hinstance);

		surface_ = instance_.createWin32SurfaceKHR(surfaceCreateInfo);
		if (!surface_) {
			throw MkException("vulkan surface creation error");
		}
	}

	const std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice_.getSurfaceFormatsKHR(surface_);
	if (surfaceFormats[0].format == vk::Format::eUndefined) {
		format_ = vk::Format::eR8G8B8A8Unorm;
	}
	else {
		format_ = surfaceFormats[0].format;
	}
	colorSpace_ = surfaceFormats[0].colorSpace;
}

void RenderEngine::initializeSwapchain(int width, int height, bool enableVsync) {

	vk::SwapchainKHR oldSwapchain = swapchain_;

	vk::SurfaceCapabilitiesKHR surfaceCaps = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
	std::vector<vk::PresentModeKHR> presentModes = physicalDevice_.getSurfacePresentModesKHR(surface_);

	vk::Extent2D swapchainExtent(kScreenWidth, kScreenHeight);
	if (surfaceCaps.currentExtent.width > -1 && surfaceCaps.currentExtent.height > -1) {
		swapchainExtent = surfaceCaps.currentExtent;
	}
	if (swapchainExtent.width > surfaceCaps.currentExtent.width) {
		swapchainExtent.width = surfaceCaps.currentExtent.width;
	}
	if (swapchainExtent.height > surfaceCaps.currentExtent.height) {
		swapchainExtent.height = surfaceCaps.currentExtent.height;
	}

	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
	if (!enableVsync) {
		size_t numPresentMode = presentModes.size();
		for (size_t i = 0; i < numPresentMode; i++) {
			if (presentModes[i] == vk::PresentModeKHR::eMailbox) {
				swapchainPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
			else if (presentModes[i] == vk::PresentModeKHR::eImmediate) {
				swapchainPresentMode = vk::PresentModeKHR::eImmediate;
			}
		}
	}

	uint32_t desiredNumSwapchainImages = surfaceCaps.minImageCount + 1;
	if ((surfaceCaps.maxImageCount > 0) && (desiredNumSwapchainImages > surfaceCaps.maxImageCount)) {
		desiredNumSwapchainImages = surfaceCaps.maxImageCount;
	}

	vk::SurfaceTransformFlagBitsKHR preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	if (!(surfaceCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)) {
		preTransform = surfaceCaps.currentTransform;
	}

	swapchainImageCount_ = desiredNumSwapchainImages;
	{
		vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(surface_)
			.setMinImageCount(desiredNumSwapchainImages)
			.setImageFormat(format_)
			.setImageColorSpace(colorSpace_)
			.setImageExtent(swapchainExtent)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
			.setPreTransform(preTransform)
			.setImageArrayLayers(1)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndexCount(0)
			.setPQueueFamilyIndices(nullptr)
			.setPresentMode(swapchainPresentMode)
			.setOldSwapchain(oldSwapchain)
			.setClipped(true)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

		swapchain_ = device_.createSwapchainKHR(swapchainCreateInfo);
		if (!swapchain_) {
			throw MkException("vulkan swapchain creation error");
		}
	}

	if (oldSwapchain) {
		for (uint32_t i = 0; i < imageCount_; i++) {
			device_.destroyImageView(imageViews_[i]);
		}

		device_.destroySwapchainKHR(oldSwapchain);
	}

	vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
		.setFormat(format_)
		.setSubresourceRange(
			vk::ImageSubresourceRange()
			.setLevelCount(1)
			.setLayerCount(1)
			.setAspectMask(vk::ImageAspectFlagBits::eColor))
		.setViewType(vk::ImageViewType::e2D);

	std::vector<vk::Image> swapchainImages = device_.getSwapchainImagesKHR(swapchain_);
	imageCount_ = (uint32_t)swapchainImages.size();

	images_.resize(imageCount_);
	imageViews_.resize(imageCount_);
	fences_.resize(imageCount_);
	for (uint32_t i = 0; i < imageCount_; i++) {
		images_[i] = swapchainImages[i];
		viewCreateInfo.setImage(swapchainImages[i]);
		imageViews_[i] = device_.createImageView(viewCreateInfo);
		fences_[i] = vk::Fence();
	}
}

void RenderEngine::cleanupSwapchain() {
	for (size_t i = 0; i < imageCount_; i++) {
		device_.destroyImageView(imageViews_[i]);
		device_.destroyFence(fences_[i]);
	}

	device_.destroySwapchainKHR(swapchain_);
	instance_.destroySurfaceKHR(surface_);
}


void RenderEngine::initializeRenderSettings() {
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo;

		presentCompleteSemaphore_ = device_.createSemaphore(semaphoreCreateInfo);
	}

	{
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			{vk::DescriptorType::eSampler,4096},
			{vk::DescriptorType::eCombinedImageSampler,4096},
			{vk::DescriptorType::eSampledImage,4096},
			{vk::DescriptorType::eStorageImage,4096},
			{vk::DescriptorType::eUniformTexelBuffer,4096},
			{vk::DescriptorType::eStorageTexelBuffer,4096},
			{vk::DescriptorType::eUniformBuffer,4096},
			{vk::DescriptorType::eStorageBuffer,4096},
			{vk::DescriptorType::eUniformBufferDynamic,4096},
			{vk::DescriptorType::eStorageBufferDynamic,4096},
			{vk::DescriptorType::eInputAttachment,4096}
		};
		vk::DescriptorPoolCreateInfo poolCreateInfo = vk::DescriptorPoolCreateInfo()
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setMaxSets(4096)
			.setPoolSizes(poolSizes);

		descriptorPool_ = device_.createDescriptorPool(poolCreateInfo);
	}

	{
		vk::AttachmentReference attachmentRefs = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::AttachmentDescription attachmentDesc = vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFormat(format_)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		vk::SubpassDescription subpassDesc = vk::SubpassDescription()
			.setColorAttachmentCount(1)
			.setPColorAttachments(&attachmentRefs)
			.setInputAttachmentCount(0)
			.setPInputAttachments(nullptr)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&attachmentDesc)
			.setDependencyCount(0)
			.setSubpassCount(1)
			.setPSubpasses(&subpassDesc);

		renderPass_ = device_.createRenderPass(renderPassCreateInfo);
	}

	{
		commandBuffers_ = allocateCommandBuffer(swapchainImageCount_);
	}

	{
		for (size_t i = 0; i < imageViews_.size(); i++) {
			vk::ImageView imageViews[1] = {
				imageViews_[i],
			};
			vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
				.setAttachmentCount(1)
				.setPAttachments(imageViews)
				.setWidth((uint32_t)kScreenWidth)
				.setHeight((uint32_t)kScreenHeight)
				.setRenderPass(renderPass_)
				.setLayers(1);

			framebuffers_.push_back(device_.createFramebuffer(framebufferCreateInfo));
		}
	}

}


void RenderEngine::cleanupRenderSettings() {
	device_.waitIdle();
	device_.destroySemaphore(presentCompleteSemaphore_);
	device_.destroyDescriptorPool(descriptorPool_);
}

uint32_t RenderEngine::acquireNextImage() {
	auto result = device_.acquireNextImageKHR(swapchain_, UINT64_MAX, presentCompleteSemaphore_);

	if (result.result != vk::Result::eSuccess) {
		throw MkException("");
	}

	currentImageIndex_ = result.value;

	return currentImageIndex_;
}

vk::Result RenderEngine::present(const std::vector<vk::Semaphore>& waitSemaphore) {
	presentInfo_.waitSemaphoreCount = (uint32_t)waitSemaphore.size();
	presentInfo_.pWaitSemaphores = waitSemaphore.data();
	presentInfo_.swapchainCount = 1;
	presentInfo_.pSwapchains = &swapchain_;
	presentInfo_.pImageIndices = &currentImageIndex_;

	return graphicsQueue_.presentKHR(presentInfo_);
}

vk::Fence RenderEngine::getSubmitFence(bool destroy) {
	auto& image = images_[currentImageIndex_];
	auto& fence = fences_[currentImageIndex_];
	while (fence) {
		vk::Result fenceRes = device_.waitForFences(fence, VK_TRUE, kFenceTimeout);
		if (fenceRes == vk::Result::eSuccess) {
			if (destroy) {
				device_.destroyFence(fence);
			}
			fence = vk::Fence();
		}
	}

	fences_[currentImageIndex_] = device_.createFence(vk::FenceCreateFlags());
	return fences_[currentImageIndex_];
}

std::vector<vk::CommandBuffer> RenderEngine::allocateCommandBuffer(uint32_t num, vk::CommandBufferLevel level) {
	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool_)
		.setCommandBufferCount(num)
		.setLevel(level);

	return device_.allocateCommandBuffers(allocInfo);
}

uint32_t RenderEngine::getMemoryTypeIndex(uint32_t bits, const vk::MemoryPropertyFlags& properties) {
	uint32_t result = 0;
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = physicalDevice_.getMemoryProperties();
	for (uint32_t i = 0; i < 32; i++) {
		if ((bits & 1) == 1) {
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
			bits >>= 1;
		}
	}

	return 0xffffffff;
}

VkBool32 DebugMessageCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* pUserData) {
	std::string message;

	std::stringstream buf;
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		buf << "ERROR: ";
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		buf << "WARNING: ";
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		buf << "PERF: ";
	}

	buf << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << "\n";
	message = buf.str();

	std::cout << message << std::endl;
	OutputDebugStringA(message.c_str());

	return false;
}
