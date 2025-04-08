#include "layer.hpp"
#include "gui.hpp"
#include "guts.hpp"

#include <memory>
#include <mutex>
#include <thread>

using scoped_lock = std::lock_guard<std::mutex>;

std::unique_ptr<impl::ShaderGuts> pShaderGuts;
std::unique_ptr<impl::Gui> pGui;
std::mutex global_lock;

// Thanks to Baldurk for the initial layer implementation.
// https://github.com/baldurk/sample_layer

///////////////////////////////////////////////////////////////////////////////////////////
// Layer init and shutdown

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_CreateInstance(
    const VkInstanceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
  VkLayerInstanceCreateInfo *layerCreateInfo =
      (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;

  // step through the chain of pNext until we get to the link info
  while (layerCreateInfo &&
         (layerCreateInfo->sType !=
              VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO ||
          layerCreateInfo->function != VK_LAYER_LINK_INFO)) {
    layerCreateInfo = (VkLayerInstanceCreateInfo *)layerCreateInfo->pNext;
  }

  if (layerCreateInfo == NULL) {
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr gpa =
      layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

  PFN_vkCreateInstance createFunc = reinterpret_cast<PFN_vkCreateInstance>(
      gpa(VK_NULL_HANDLE, "vkCreateInstance"));

  VkResult ret = createFunc(pCreateInfo, pAllocator, pInstance);

  VkLayerInstanceDispatchTable dispatchTable;
  dispatchTable.GetInstanceProcAddr =
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(
          gpa(*pInstance, "vkGetInstanceProcAddr"));
  dispatchTable.DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
      gpa(*pInstance, "vkDestroyInstance"));
  dispatchTable.EnumerateDeviceExtensionProperties =
      reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(
          gpa(*pInstance, "vkEnumerateDeviceExtensionProperties"));

  {
    scoped_lock l(global_lock);
    pShaderGuts = std::make_unique<impl::ShaderGuts>(impl::ShaderGuts());
    pGui = std::make_unique<impl::Gui>(impl::Gui(*pShaderGuts));
    std::thread t([&]() { pGui->Draw(); });
    t.detach();

    instance_dispatch[GetKey(*pInstance)] = dispatchTable;
  }

  return VK_SUCCESS;
}

VK_LAYER_EXPORT void VKAPI_CALL ShaderGuts_DestroyInstance(
    VkInstance instance, const VkAllocationCallbacks *pAllocator) {
  scoped_lock l(global_lock);
  instance_dispatch.erase(GetKey(instance));
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_CreateDevice(
    VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
  VkLayerDeviceCreateInfo *layerCreateInfo =
      (VkLayerDeviceCreateInfo *)pCreateInfo->pNext;

  while (layerCreateInfo && (layerCreateInfo->sType !=
                                 VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO ||
                             layerCreateInfo->function != VK_LAYER_LINK_INFO)) {
    layerCreateInfo = (VkLayerDeviceCreateInfo *)layerCreateInfo->pNext;
  }

  if (layerCreateInfo == NULL) {
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr gipa =
      layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  PFN_vkGetDeviceProcAddr gdpa =
      layerCreateInfo->u.pLayerInfo->pfnNextGetDeviceProcAddr;
  // move chain on for next layer
  layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

  PFN_vkCreateDevice createFunc = reinterpret_cast<PFN_vkCreateDevice>(
      gipa(VK_NULL_HANDLE, "vkCreateDevice"));

  VkResult ret = createFunc(physicalDevice, pCreateInfo, pAllocator, pDevice);

  VkLayerDispatchTable dispatchTable;

  dispatchTable.GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
      gdpa(*pDevice, "vkGetDeviceProcAddr"));
  dispatchTable.DestroyDevice =
      reinterpret_cast<PFN_vkDestroyDevice>(gdpa(*pDevice, "vkDestroyDevice"));
  dispatchTable.CreateComputePipelines =
      reinterpret_cast<PFN_vkCreateComputePipelines>(
          gdpa(*pDevice, "vkCreateComputePipelines"));
  dispatchTable.CreateGraphicsPipelines =
      reinterpret_cast<PFN_vkCreateGraphicsPipelines>(
          gdpa(*pDevice, "vkCreateGraphicsPipelines"));
  dispatchTable.CreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(
      gdpa(*pDevice, "vkCreateShaderModule"));
  dispatchTable.CreateShadersEXT =
      (PFN_vkCreateShadersEXT)gdpa(*pDevice, "vkCreateShadersEXT");

  dispatchTable.QueuePresentKHR =
      (PFN_vkQueuePresentKHR)gdpa(*pDevice, "vkQueuePresentKHR");
  dispatchTable.AcquireNextImageKHR =
      (PFN_vkAcquireNextImageKHR)gdpa(*pDevice, "vkAcquireNextImageKHR");
  {
    scoped_lock l(global_lock);
    device_dispatch[GetKey(*pDevice)] = dispatchTable;
  }

  return VK_SUCCESS;
}

VK_LAYER_EXPORT void VKAPI_CALL ShaderGuts_DestroyDevice(
    VkDevice device, const VkAllocationCallbacks *pAllocator) {
  scoped_lock l(global_lock);

  device_dispatch.erase(GetKey(device));
}

///////////////////////////////////////////////////////////////////////////////////////////
// Actual layer implementation

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_CreateShaderModule(
    VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule) {
  scoped_lock l(global_lock);
  pShaderGuts->CreateShaderModulePre(pCreateInfo);
  auto ret = device_dispatch[GetKey(device)].CreateShaderModule(
      device, pCreateInfo, pAllocator, pShaderModule);
  pShaderGuts->CreateShaderModulePost(pCreateInfo, pShaderModule);
  return ret;
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_CreateShadersEXT(
    VkDevice device, uint32_t createInfoCount,
    const VkShaderCreateInfoEXT *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders) {
  scoped_lock l(global_lock);
  pShaderGuts->CreateShadersEXT(createInfoCount, pCreateInfos);
  return device_dispatch[GetKey(device)].CreateShadersEXT(
      device, createInfoCount, pCreateInfos, pAllocator, pShaders);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_CreateGraphicsPipelines(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkGraphicsPipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
  scoped_lock l(global_lock);
  pShaderGuts->CreateGraphicsPipelines(createInfoCount, pCreateInfos);
  return device_dispatch[GetKey(device)].CreateGraphicsPipelines(
      device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
      pPipelines);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_CreateComputePipelines(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkComputePipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
  scoped_lock l(global_lock);
  pShaderGuts->CreateComputePipelines(createInfoCount, pCreateInfos);
  return device_dispatch[GetKey(device)].CreateComputePipelines(
      device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
      pPipelines);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_AcquireNextImageKHR(
    VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) {
  scoped_lock l(global_lock);
  pShaderGuts->AcquireNextImageKHR();
  return device_dispatch[GetKey(device)].AcquireNextImageKHR(
      device, swapchain, timeout, semaphore, fence, pImageIndex);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_QueuePresentKHR(
    VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
  scoped_lock l(global_lock);
  pShaderGuts->QueuePresentKHR();
  return device_dispatch[GetKey(queue)].QueuePresentKHR(queue, pPresentInfo);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Enumeration function

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_EnumerateInstanceLayerProperties(
    uint32_t *pPropertyCount, VkLayerProperties *pProperties) {
  if (pPropertyCount)
    *pPropertyCount = 1;

  if (pProperties) {
    strcpy(pProperties->layerName, "VK_LAYER_shader_guts");
    strcpy(pProperties->description, "Dump and load shaders.");
    pProperties->implementationVersion = 1;
    pProperties->specVersion = VK_API_VERSION_1_0;
  }

  return VK_SUCCESS;
}

VK_LAYER_EXPORT VkResult VKAPI_CALL ShaderGuts_EnumerateDeviceLayerProperties(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
    VkLayerProperties *pProperties) {
  return ShaderGuts_EnumerateInstanceLayerProperties(pPropertyCount,
                                                     pProperties);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL
ShaderGuts_EnumerateInstanceExtensionProperties(
    const char *pLayerName, uint32_t *pPropertyCount,
    VkExtensionProperties *pProperties) {
  if (pLayerName == nullptr || strcmp(pLayerName, "VK_LAYER_shader_guts"))
    return VK_ERROR_LAYER_NOT_PRESENT;

  // don't expose any extensions
  if (pPropertyCount)
    *pPropertyCount = 0;
  return VK_SUCCESS;
}

VK_LAYER_EXPORT VkResult VKAPI_CALL
ShaderGuts_EnumerateDeviceExtensionProperties(
    VkPhysicalDevice physicalDevice, const char *pLayerName,
    uint32_t *pPropertyCount, VkExtensionProperties *pProperties) {
  // pass through any queries that aren't to us
  if (pLayerName == nullptr || strcmp(pLayerName, "VK_LAYER_shader_guts")) {
    if (physicalDevice == VK_NULL_HANDLE)
      return VK_SUCCESS;

    scoped_lock l(global_lock);
    return instance_dispatch[GetKey(physicalDevice)]
        .EnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                            pPropertyCount, pProperties);
  }

  // don't expose any extensions
  if (pPropertyCount)
    *pPropertyCount = 0;
  return VK_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////
// GetProcAddr functions, entry points of the layer

#define GETPROCADDR(func)                                                      \
  if (!strcmp(pName, "vk" #func))                                              \
    return (PFN_vkVoidFunction) & ShaderGuts_##func;

VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL
ShaderGuts_GetDeviceProcAddr(VkDevice device, const char *pName) {
  // device chain functions we intercept
  GETPROCADDR(GetDeviceProcAddr);
  GETPROCADDR(EnumerateDeviceLayerProperties);
  GETPROCADDR(EnumerateDeviceExtensionProperties);
  GETPROCADDR(CreateDevice);
  GETPROCADDR(DestroyDevice);

  GETPROCADDR(CreateComputePipelines);
  GETPROCADDR(CreateGraphicsPipelines);

  GETPROCADDR(CreateShaderModule);
  GETPROCADDR(CreateShadersEXT);

  GETPROCADDR(AcquireNextImageKHR);
  GETPROCADDR(QueuePresentKHR);

  {
    scoped_lock l(global_lock);
    return device_dispatch[GetKey(device)].GetDeviceProcAddr(device, pName);
  }
}

VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL
ShaderGuts_GetInstanceProcAddr(VkInstance instance, const char *pName) {
  // instance chain functions we intercept
  GETPROCADDR(GetInstanceProcAddr);
  GETPROCADDR(EnumerateInstanceLayerProperties);
  GETPROCADDR(EnumerateInstanceExtensionProperties);
  GETPROCADDR(CreateInstance);
  GETPROCADDR(DestroyInstance);

  // device chain functions we intercept
  GETPROCADDR(GetDeviceProcAddr);
  GETPROCADDR(EnumerateDeviceLayerProperties);
  GETPROCADDR(EnumerateDeviceExtensionProperties);
  GETPROCADDR(CreateDevice);
  GETPROCADDR(DestroyDevice);

  {
    scoped_lock l(global_lock);
    return instance_dispatch[GetKey(instance)].GetInstanceProcAddr(instance,
                                                                   pName);
  }
}