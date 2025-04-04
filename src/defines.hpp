#pragma once
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan.h>

#undef VK_LAYER_EXPORT
#if defined(WIN32)
#define VK_LAYER_EXPORT extern "C" __declspec(dllexport)
#else
#define VK_LAYER_EXPORT extern "C"
#endif

typedef struct VkLayerDispatchTable_ {
  PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
  PFN_vkDestroyDevice DestroyDevice;
  PFN_vkGetDeviceQueue GetDeviceQueue;
  PFN_vkQueueSubmit QueueSubmit;
  PFN_vkQueueWaitIdle QueueWaitIdle;
  PFN_vkDeviceWaitIdle DeviceWaitIdle;
  PFN_vkAllocateMemory AllocateMemory;
  PFN_vkFreeMemory FreeMemory;
  PFN_vkMapMemory MapMemory;
  PFN_vkUnmapMemory UnmapMemory;
  PFN_vkFlushMappedMemoryRanges FlushMappedMemoryRanges;
  PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges;
  PFN_vkGetDeviceMemoryCommitment GetDeviceMemoryCommitment;
  PFN_vkGetImageSparseMemoryRequirements GetImageSparseMemoryRequirements;
  PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
  PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
  PFN_vkBindImageMemory BindImageMemory;
  PFN_vkBindBufferMemory BindBufferMemory;
  PFN_vkQueueBindSparse QueueBindSparse;
  PFN_vkCreateFence CreateFence;
  PFN_vkDestroyFence DestroyFence;
  PFN_vkGetFenceStatus GetFenceStatus;
  PFN_vkResetFences ResetFences;
  PFN_vkWaitForFences WaitForFences;
  PFN_vkCreateSemaphore CreateSemaphore;
  PFN_vkDestroySemaphore DestroySemaphore;
  PFN_vkCreateEvent CreateEvent;
  PFN_vkDestroyEvent DestroyEvent;
  PFN_vkGetEventStatus GetEventStatus;
  PFN_vkSetEvent SetEvent;
  PFN_vkResetEvent ResetEvent;
  PFN_vkCreateQueryPool CreateQueryPool;
  PFN_vkDestroyQueryPool DestroyQueryPool;
  PFN_vkGetQueryPoolResults GetQueryPoolResults;
  PFN_vkCreateBuffer CreateBuffer;
  PFN_vkDestroyBuffer DestroyBuffer;
  PFN_vkCreateBufferView CreateBufferView;
  PFN_vkDestroyBufferView DestroyBufferView;
  PFN_vkCreateImage CreateImage;
  PFN_vkDestroyImage DestroyImage;
  PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout;
  PFN_vkCreateImageView CreateImageView;
  PFN_vkDestroyImageView DestroyImageView;
  PFN_vkCreateShaderModule CreateShaderModule;
  PFN_vkDestroyShaderModule DestroyShaderModule;
  PFN_vkCreatePipelineCache CreatePipelineCache;
  PFN_vkDestroyPipelineCache DestroyPipelineCache;
  PFN_vkGetPipelineCacheData GetPipelineCacheData;
  PFN_vkMergePipelineCaches MergePipelineCaches;
  PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines;
  PFN_vkCreateComputePipelines CreateComputePipelines;
  PFN_vkDestroyPipeline DestroyPipeline;
  PFN_vkCreatePipelineLayout CreatePipelineLayout;
  PFN_vkDestroyPipelineLayout DestroyPipelineLayout;
  PFN_vkCreateSampler CreateSampler;
  PFN_vkDestroySampler DestroySampler;
  PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout;
  PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout;
  PFN_vkCreateDescriptorPool CreateDescriptorPool;
  PFN_vkDestroyDescriptorPool DestroyDescriptorPool;
  PFN_vkResetDescriptorPool ResetDescriptorPool;
  PFN_vkAllocateDescriptorSets AllocateDescriptorSets;
  PFN_vkFreeDescriptorSets FreeDescriptorSets;
  PFN_vkUpdateDescriptorSets UpdateDescriptorSets;
  PFN_vkCreateFramebuffer CreateFramebuffer;
  PFN_vkDestroyFramebuffer DestroyFramebuffer;
  PFN_vkCreateRenderPass CreateRenderPass;
  PFN_vkDestroyRenderPass DestroyRenderPass;
  PFN_vkGetRenderAreaGranularity GetRenderAreaGranularity;
  PFN_vkCreateCommandPool CreateCommandPool;
  PFN_vkDestroyCommandPool DestroyCommandPool;
  PFN_vkResetCommandPool ResetCommandPool;
  PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
  PFN_vkFreeCommandBuffers FreeCommandBuffers;
  PFN_vkBeginCommandBuffer BeginCommandBuffer;
  PFN_vkEndCommandBuffer EndCommandBuffer;
  PFN_vkResetCommandBuffer ResetCommandBuffer;
  PFN_vkCmdBindPipeline CmdBindPipeline;
  PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
  PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers;
  PFN_vkCmdBindIndexBuffer CmdBindIndexBuffer;
  PFN_vkCmdSetViewport CmdSetViewport;
  PFN_vkCmdSetScissor CmdSetScissor;
  PFN_vkCmdSetLineWidth CmdSetLineWidth;
  PFN_vkCmdSetDepthBias CmdSetDepthBias;
  PFN_vkCmdSetBlendConstants CmdSetBlendConstants;
  PFN_vkCmdSetDepthBounds CmdSetDepthBounds;
  PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask;
  PFN_vkCmdSetStencilWriteMask CmdSetStencilWriteMask;
  PFN_vkCmdSetStencilReference CmdSetStencilReference;
  PFN_vkCmdDraw CmdDraw;
  PFN_vkCmdDrawIndexed CmdDrawIndexed;
  PFN_vkCmdDrawIndirect CmdDrawIndirect;
  PFN_vkCmdDrawIndexedIndirect CmdDrawIndexedIndirect;
  PFN_vkCmdDispatch CmdDispatch;
  PFN_vkCmdDispatchIndirect CmdDispatchIndirect;
  PFN_vkCmdCopyBuffer CmdCopyBuffer;
  PFN_vkCmdCopyImage CmdCopyImage;
  PFN_vkCmdBlitImage CmdBlitImage;
  PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
  PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer;
  PFN_vkCmdUpdateBuffer CmdUpdateBuffer;
  PFN_vkCmdFillBuffer CmdFillBuffer;
  PFN_vkCmdClearColorImage CmdClearColorImage;
  PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage;
  PFN_vkCmdClearAttachments CmdClearAttachments;
  PFN_vkCmdResolveImage CmdResolveImage;
  PFN_vkCmdSetEvent CmdSetEvent;
  PFN_vkCmdResetEvent CmdResetEvent;
  PFN_vkCmdWaitEvents CmdWaitEvents;
  PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
  PFN_vkCmdBeginQuery CmdBeginQuery;
  PFN_vkCmdEndQuery CmdEndQuery;
  PFN_vkCmdResetQueryPool CmdResetQueryPool;
  PFN_vkCmdWriteTimestamp CmdWriteTimestamp;
  PFN_vkCmdCopyQueryPoolResults CmdCopyQueryPoolResults;
  PFN_vkCmdPushConstants CmdPushConstants;
  PFN_vkCmdBeginRenderPass CmdBeginRenderPass;
  PFN_vkCmdNextSubpass CmdNextSubpass;
  PFN_vkCmdEndRenderPass CmdEndRenderPass;
  PFN_vkCmdExecuteCommands CmdExecuteCommands;
  PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
  PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
  PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
  PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
  PFN_vkQueuePresentKHR QueuePresentKHR;
  PFN_vkCmdDrawIndirectCountAMD CmdDrawIndirectCountAMD;
  PFN_vkCmdDrawIndexedIndirectCountAMD CmdDrawIndexedIndirectCountAMD;
#ifdef VK_USE_PLATFORM_WIN32_KHR
  PFN_vkGetMemoryWin32HandleNV GetMemoryWin32HandleNV;
#endif
  PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR;
  PFN_vkDebugMarkerSetObjectTagEXT DebugMarkerSetObjectTagEXT;
  PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT;
  PFN_vkCmdDebugMarkerBeginEXT CmdDebugMarkerBeginEXT;
  PFN_vkCmdDebugMarkerEndEXT CmdDebugMarkerEndEXT;
  PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT;

  PFN_vkCreateShadersEXT CreateShadersEXT;
  PFN_vkGetShaderBinaryDataEXT GetShaderBinaryDataEXT;
} VkLayerDispatchTable;

typedef struct VkLayerInstanceDispatchTable_ {
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
  PFN_vkDestroyInstance DestroyInstance;
  PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
  PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures;
  PFN_vkGetPhysicalDeviceImageFormatProperties
      GetPhysicalDeviceImageFormatProperties;
  PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties
      GetPhysicalDeviceSparseImageFormatProperties;
  PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties
      GetPhysicalDeviceQueueFamilyProperties;
  PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
  PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
  PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties;
  PFN_vkDestroySurfaceKHR DestroySurfaceKHR;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
      GetPhysicalDeviceSurfaceCapabilitiesKHR;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
      GetPhysicalDeviceSurfacePresentModesKHR;
  PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT;
  PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT;
  PFN_vkDebugReportMessageEXT DebugReportMessageEXT;
#ifdef VK_USE_PLATFORM_MIR_KHR
  PFN_vkCreateMirSurfaceKHR CreateMirSurfaceKHR;
  PFN_vkGetPhysicalDeviceMirPresentationSupportKHR
      GetPhysicalDeviceMirPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
  PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR;
  PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR
      GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
  PFN_vkCreateWin32SurfaceKHR CreateWin32SurfaceKHR;
  PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR
      GetPhysicalDeviceWin32PresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
  PFN_vkCreateXcbSurfaceKHR CreateXcbSurfaceKHR;
  PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR
      GetPhysicalDeviceXcbPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
  PFN_vkCreateXlibSurfaceKHR CreateXlibSurfaceKHR;
  PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR
      GetPhysicalDeviceXlibPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
  PFN_vkCreateAndroidSurfaceKHR CreateAndroidSurfaceKHR;
#endif
  PFN_vkGetPhysicalDeviceDisplayPropertiesKHR
      GetPhysicalDeviceDisplayPropertiesKHR;
  PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR
      GetPhysicalDeviceDisplayPlanePropertiesKHR;
  PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR;
  PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR;
  PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR;
  PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR;
  PFN_vkCreateDisplayPlaneSurfaceKHR CreateDisplayPlaneSurfaceKHR;
  PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV
      GetPhysicalDeviceExternalImageFormatPropertiesNV;
} VkLayerInstanceDispatchTable;