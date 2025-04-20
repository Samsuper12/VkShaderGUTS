#pragma once

#include "defines.hpp"

#include <algorithm>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <utility>
#include <vector>

class PipelineLibrary {
public:
  enum class Type { Graphics, Compute, ShaderObjectEXT };

  static inline auto TypeToString(Type t) noexcept -> std::string {
    switch (t) {
    case Type::Graphics:
      return "Graphics";
    case Type::Compute:
      return "Compute";
    case Type::ShaderObjectEXT:
      return "ShaderObjectEXT";
    }
    std::unreachable();
  };

  struct Pipeline {

    struct Data {
      bool edited;
      bool result;
      uint32_t index;
      Type type;
      float compileDuration;
    };

    VkPipeline originPipeline;
    // TODO:
    VkPipeline editedPipeline;

    Pipeline(Type t, float duration, VkPipeline pipeline) {
      data.index = GenIndex();
      data.compileDuration = duration;
      data.type = t;
      originPipeline = pipeline;
    }

    Data data{};

  protected:
    uint32_t GenIndex() {
      static uint32_t index = 0;
      return ++index;
    }
  };

  PipelineLibrary() : canPull(true) {}

  PipelineLibrary(const PipelineLibrary &) = delete;
  PipelineLibrary(PipelineLibrary &) = delete;
  PipelineLibrary &operator=(const PipelineLibrary &) = delete;

  PipelineLibrary(PipelineLibrary &&other) {
    std::unique_lock lock(allMutex);
    allPipelines = other.allPipelines;
    usedPipelines = other.usedPipelines;
    canPull = other.canPull;
  }

  template <typename VkInfo>
  auto AddPipeline(VkResult result, float duration, PipelineLibrary::Type type,
                   uint32_t createInfoCount, const VkInfo *pCreateInfos,
                   VkPipeline *pPipeline) -> void {
    std::unique_lock lock(allMutex);

    allPipelines.push_back(Pipeline(type, duration, *pPipeline));
    canPull = false;
  }

  auto MarkUsed(VkPipeline pipeline) -> void {
    std::shared_lock lock(allMutex);

    auto comp = [&](const Pipeline &p) -> bool {
      return pipeline == p.originPipeline;
    };

    // FIXME:
    if (auto f1 = std::find_if(allPipelines.begin(), allPipelines.end(), comp);
        f1 != allPipelines.end()) {
      if (auto f2 =
              std::find_if(usedPipelines.begin(), usedPipelines.end(), comp);
          f2 == usedPipelines.end()) {
        usedPipelines.push_back(*f1);
      }
      canPull = false;
    }
  }

  auto EndOfFrame() -> void { canPull = true; }

  auto ResetUsed() -> void {
    std::unique_lock lock(allMutex);
    usedPipelines.clear();
    canPull = false;
  }

  auto GetAllPipelines() const -> std::vector<Pipeline> {
    std::shared_lock lock(allMutex);
    return allPipelines;
  }

  auto GetLastFramePipelines() -> std::vector<Pipeline> {
    std::shared_lock lock(allMutex);
    return usedPipelines | std::ranges::to<std::vector<Pipeline>>();
  }

  auto ReadyToPull() const -> bool { return canPull; }

private:
  bool canPull;
  std::vector<Pipeline> allPipelines;
  std::vector<Pipeline> usedPipelines;

  mutable std::shared_mutex allMutex;
};