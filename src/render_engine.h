﻿#ifndef _RENDER_ENGINE_H_
#define _RENDER_ENGINE_H_

#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vk_platform.h>

class RenderEngine {
private:
	friend class RenderPipeline;

public:
	~RenderEngine() {}

	void initializeContext();
	void cleanupContext();

	void initializeSurface(HWND hwnd, HINSTANCE hinstance);
	void initializeSwapchain(int width, int height, bool enableVsync);
	void cleanupSwapchain();

	void initializeRenderSettings();
	void cleanupRenderSettings();

	uint32_t acquireNextImage();
	vk::Result present(const std::vector<vk::Semaphore>& waitSemaphore);
	vk::Fence getSubmitFence(bool destroy = false);

	std::vector<vk::CommandBuffer> allocateCommandBuffer(uint32_t num);

private:
	vk::Instance instance_;
	std::vector<vk::PhysicalDevice> physicalDevices_;
	vk::PhysicalDevice physicalDevice_;
	vk::Device device_;

	vk::Queue graphicsQueue_;

	vk::PipelineCache pipelineCache_;

	vk::SurfaceKHR surface_;
	vk::SwapchainKHR swapchain_;
	vk::PresentInfoKHR presentInfo_;

	vk::Format format_;
	vk::ColorSpaceKHR colorSpace_;
	uint32_t imageCount_;

	std::vector<vk::Image> images_;
	std::vector<vk::ImageView> imageViews_;
	std::vector<vk::Fence> fences_;

	std::vector<vk::Framebuffer> framebuffers_;

	vk::CommandPool commandPool_;
	std::vector<vk::CommandBuffer> commandBuffers_;

	vk::Semaphore presentCompleteSemaphore_;
	vk::Semaphore renderCompleteSemaphore_;
	uint32_t currentImageIndex_;

	static const uint64_t kFenceTimeout = 1000000000000;

#if defined(_DEBUG)
	PFN_vkCreateDebugReportCallbackEXT createDebugreportCallback = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportCallback = VK_NULL_HANDLE;
	PFN_vkDebugReportMessageEXT debugBreakCallback = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT msgCallback;
#endif
};

VkBool32 DebugMessageCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* pUserData);

#endif