#pragma once
#include "defines.hpp"
#include "glslangShaders.hpp"
#include "util.hpp"
#include <chrono>
#include <cstdint>
#include <sys/types.h>
#include <thread>

namespace impl {
class ShaderGuts {
public:
  enum class ShaderLanguage { spirv, glsl };

  struct Playback {
    bool play = true;
    uint step = 0;
    uint64_t frameCount = 0;
  };

  ShaderGuts()
      : dumpEnable(false), loadEnable(false), loadLang(ShaderLanguage::spirv) {
    namespace fs = std::filesystem;

    bool dump = util::envContainsString("VK_SHADER_GUTS_DUMP_PATH", dumpPath);
    util::envContains<ShaderLanguage>("VK_SHADER_GUTS_DUMP_LANG",
                                      stringToSourceType, dumpLang);

    bool load = util::envContainsString("VK_SHADER_GUTS_LOAD_PATH", loadPath);
    bool hash = util::envContainsString("VK_SHADER_GUTS_LOAD_HASH", loadHash);
    util::envContains<ShaderLanguage>("VK_SHADER_GUTS_LOAD_LANG",
                                      stringToSourceType, loadLang);

    if (dump)
      dumpEnable = fs::exists(dumpPath) ? true : fs::create_directory(dumpPath);

    if (load && hash)
      loadEnable = hash && fs::exists(loadPath);

    PrintLogs();
  }

  auto SetPlayback(bool value) -> void { playback.play = value; }
  auto SetPlayStep() -> void { playback.step++; }
  auto GetFrameCount() const -> int64_t { return playback.frameCount; }

  auto AcquireNextImageKHR() -> void {

    while (!playback.play) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      if (playback.step > 0)
        return;
    }
  };
  auto QueuePresentKHR() -> void {
    if (playback.step > 0)
      playback.step--;

    playback.frameCount++;
  };

  auto CreateShaderModulePre(const VkShaderModuleCreateInfo *pCreateInfo)
      -> void {
    using sourceType = uint32_t;

    if (!loadEnable)
      return;

    LoadShader<sourceType, VkShaderModuleCreateInfo>(pCreateInfo);
  }

  auto CreateShaderModulePost(const VkShaderModuleCreateInfo *pCreateInfo,
                              VkShaderModule *pShaderModule) -> void {
    if (!dumpEnable)
      return;

    auto shaderCode = std::vector<std::byte>(pCreateInfo->codeSize);
    std::memcpy(shaderCode.data(), pCreateInfo->pCode, pCreateInfo->codeSize);

    shaderModules[*pShaderModule] = shaderCode;
  }

  auto CreateShadersEXT(uint32_t createInfoCount,
                        const VkShaderCreateInfoEXT *pCreateInfos) -> void {
    using sourceType = std::byte;

    if (!(dumpEnable || loadEnable))
      return;

    for (size_t i = 0; i < createInfoCount; ++i) {
      auto constShaderInfo = &pCreateInfos[i];

      if (dumpEnable)
        DumpShader2<VkShaderCreateInfoEXT>(constShaderInfo,
                                           constShaderInfo->stage);

      if (loadEnable)
        LoadShader<sourceType, VkShaderCreateInfoEXT>(constShaderInfo);
    }
  }

  auto CreateGraphicsPipelines(uint32_t createInfoCount,
                               const VkGraphicsPipelineCreateInfo *pCreateInfos)
      -> void {
    using sourceType = uint32_t;

    if (!(dumpEnable || loadEnable))
      return;

    for (size_t i = 0; i < createInfoCount; i++) {
      for (size_t j = 0; j < pCreateInfos[i].stageCount; j++) {
        auto stage = pCreateInfos[i].pStages[j];

        auto *next = reinterpret_cast<const VkBaseInStructure *>(stage.pNext);

        while (next) {
          if (next->sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO) {
            auto constModuleInfo =
                reinterpret_cast<const VkShaderModuleCreateInfo *>(next);

            if (dumpEnable)
              DumpShader2<VkShaderModuleCreateInfo>(constModuleInfo,
                                                    stage.stage);

            if (loadEnable)
              LoadShader<sourceType, VkShaderModuleCreateInfo>(constModuleInfo);
          }
          next = reinterpret_cast<const VkBaseInStructure *>(next->pNext);
        }

        if (dumpEnable && shaderModules.contains(stage.module)) {
          DumpShader(shaderModules[stage.module], stage.stage);
        }
      }
    }
  }

  auto CreateComputePipelines(uint32_t createInfoCount,
                              const VkComputePipelineCreateInfo *pCreateInfos) {
    using sourceType = uint32_t;

    if (!(dumpEnable || loadEnable))
      return;

    for (size_t i = 0; i < createInfoCount; i++) {
      auto *next = reinterpret_cast<const VkBaseInStructure *>(
          pCreateInfos[i].stage.pNext);

      while (next) {
        if (next->sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO) {
          auto constModuleInfo =
              reinterpret_cast<const VkShaderModuleCreateInfo *>(next);

          if (dumpEnable)
            DumpShader2<VkShaderModuleCreateInfo>(constModuleInfo,
                                                  pCreateInfos[i].stage.stage);

          if (loadEnable)
            LoadShader<sourceType, VkShaderModuleCreateInfo>(constModuleInfo);
        }
        next = reinterpret_cast<const VkBaseInStructure *>(next->pNext);
      }

      auto stage = pCreateInfos[i].stage;
      if (dumpEnable && shaderModules.contains(stage.module)) {
        DumpShader(shaderModules[stage.module], stage.stage);
      }
    }
  }

protected:
  template <typename T, typename CreateInfo>
  auto LoadShader(const CreateInfo *info) -> void {
    if (util::Sha1Hash::compute(
            reinterpret_cast<const std::byte *>(info->pCode),
            info->codeSize * sizeof(std::byte))
            .toString() != loadHash)
      return;

    auto shaderInfo = const_cast<CreateInfo *>(info);
    switch (loadLang) {
    case ShaderLanguage::spirv:
      currentShader = util::LoadSPRV(loadPath);
      break;

    case ShaderLanguage::glsl:
      auto glslShader = util::LoadFile(loadPath);
      auto glslType = util::shaders::findShaderType(loadPath);
      auto glslVersion = util::shaders::findGLSLVersion(glslShader);

      if (!glslType) {
        std::cerr << glslType.error();
        return;
      }
      if (!glslVersion) {
        std::cerr << glslVersion.error();
        return;
      }

      currentShader = util::shaders::compileGLSL(
          {glslShader, glslVersion.value(), glslType.value()});

      break;
    }

    shaderInfo->pCode = reinterpret_cast<T *>(currentShader.data());
    shaderInfo->codeSize = currentShader.size();
  }

  template <typename CreateInfo>
  auto DumpShader2(const CreateInfo *info, const VkShaderStageFlagBits stage)
      -> void {
    auto shaderCode = std::vector<std::byte>(info->codeSize);
    std::memcpy(shaderCode.data(), info->pCode, info->codeSize);
    DumpShader(shaderCode, stage);
  }

  auto DumpShader(const std::vector<std::byte> &shader,
                  const VkShaderStageFlagBits stage) -> void {
    const auto folder = std::string("/" + stageToName[stage] + "/");
    const auto hash =
        util::Sha1Hash::compute(shader.data(), shader.size()).toString();

    if (!std::filesystem::exists(this->dumpPath + folder))
      std::filesystem::create_directory(dumpPath + folder);

    switch (dumpLang) {
    case ShaderLanguage::glsl:
      util::SaveGLSLToFile(
          shader, {dumpPath + folder + hash + "." + stageToFileExt[stage]});
      break;
    case ShaderLanguage::spirv:
      util::SaveSPVToFile<std::byte>(shader,
                                     {dumpPath + folder + hash + ".glsl"});
      break;
    }
  }

  auto PrintLogs() -> void {

    if (dumpEnable) {
      std::clog << "[VK_SHADER_GUTS][log]: VK_SHADER_GUTS_DUMP_PATH = "
                << dumpPath << "\n";
    }

    if (loadEnable) {
      std::clog << "[VK_SHADER_GUTS][log]: VK_SHADER_GUTS_LOAD_PATH = "
                << loadPath << "\n";
      std::clog << "[VK_SHADER_GUTS][log]: VK_SHADER_GUTS_LOAD_HASH = "
                << loadHash << "\n";
    }

    // FIXME:
    if (loadLang != ShaderLanguage::spirv) {
      std::clog << "[VK_SHADER_GUTS][log]: VK_SHADER_GUTS_LOAD_LANG = glsl \n";
    }
  }

private:
  bool dumpEnable;
  bool loadEnable;
  std::string dumpPath;
  std::string loadPath;
  std::string loadHash;

  ShaderLanguage dumpLang;
  ShaderLanguage loadLang;

  // Keep the shader until it loads up into the driver.
  std::vector<std::byte> currentShader;

  std::map<VkShaderModule, std::vector<std::byte>> shaderModules;
  std::map<VkShaderStageFlagBits, std::string> stageToName{
      {VK_SHADER_STAGE_VERTEX_BIT, "VS"},
      {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "TS_Control"},
      {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "TS_evaluation"},
      {VK_SHADER_STAGE_GEOMETRY_BIT, "GS"},
      {VK_SHADER_STAGE_FRAGMENT_BIT, "FS"},
      {VK_SHADER_STAGE_COMPUTE_BIT, "CS"},
  };

  std::map<VkShaderStageFlagBits, std::string> stageToFileExt{
      {VK_SHADER_STAGE_VERTEX_BIT, "vert"},
      {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesc"},
      {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tese"},
      {VK_SHADER_STAGE_GEOMETRY_BIT, "geom"},
      {VK_SHADER_STAGE_FRAGMENT_BIT, "frag"},
      {VK_SHADER_STAGE_COMPUTE_BIT, "comp"},
  };
  std::map<std::string_view, ShaderLanguage> stringToSourceType{
      {"spirv", ShaderLanguage::spirv}, {"glsl", ShaderLanguage::glsl}};

  Playback playback;
};

}; // namespace impl