#pragma once
#include "defines.hpp"
#include <map>

inline std::map<void *, VkLayerInstanceDispatchTable> instance_dispatch;
inline std::map<void *, VkLayerDispatchTable> device_dispatch;

template <typename DispatchableType> void *GetKey(DispatchableType inst) {
  return *reinterpret_cast<void **>(inst);
}