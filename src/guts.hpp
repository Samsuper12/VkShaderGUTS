#pragma once
#include "defines.hpp"
#include "glslangShaders.hpp"
#include "util.hpp"

namespace impl {
class ShaderGuts {
public:
  enum class LoadSourceType { spirv, glsl };

  ShaderGuts()
      : dumpEnable(false), loadEnable(false), loadType(LoadSourceType::spirv) {
    namespace fs = std::filesystem;

    bool dump = util::envContainsString("VK_SHADER_GUTS_DUMP_PATH", dumpPath);
    bool load = util::envContainsString("VK_SHADER_GUTS_LOAD_PATH", loadPath);
    bool hash = util::envContainsString("VK_SHADER_GUTS_LOAD_HASH", loadHash);
    util::envContains<LoadSourceType>("VK_SHADER_GUTS_LOAD_TYPE",
                                      stringToSourceType, loadType);

    if (dump)
      dumpEnable = fs::exists(dumpPath) ? true : fs::create_directory(dumpPath);

    if (load && hash)
      loadEnable = hash && fs::exists(loadPath);

    PrintLogs();
  }

  auto CreateShaderModulePre(const VkShaderModuleCreateInfo *pCreateInfo)
      -> void {
    using sourceType = uint32_t;

    if (!loadEnable)
      return;

    LoadShader<sourceType, VkShaderModuleCreateInfo>(pCreateInfo);
  }

  auto CreateShaderModulePost(const VkShaderModuleCreateInfo *pCreateInfo,
                              VkShaderModule *pShaderModule) -> void {
    using sourceType = uint32_t;

    if (!dumpEnable)
      return;

    shaderModules[*pShaderModule] = std::vector<sourceType>(
        pCreateInfo->pCode,
        pCreateInfo->pCode + (pCreateInfo->codeSize / sizeof(sourceType)));
  }

  auto CreateShadersEXT(uint32_t createInfoCount,
                        const VkShaderCreateInfoEXT *pCreateInfos) -> void {
    using sourceType = std::byte;

    if (!(dumpEnable || loadEnable))
      return;

    for (size_t i = 0; i < createInfoCount; ++i) {
      auto constShaderInfo = &pCreateInfos[i];

      if (dumpEnable)
        DumpShader2<sourceType, VkShaderCreateInfoEXT>(
            constShaderInfo, stageToName[constShaderInfo->stage]);

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
              DumpShader2<sourceType, VkShaderModuleCreateInfo>(
                  constModuleInfo, stageToName[stage.stage]);

            if (loadEnable)
              LoadShader<sourceType, VkShaderModuleCreateInfo>(constModuleInfo);
          }
          next = reinterpret_cast<const VkBaseInStructure *>(next->pNext);
        }

        if (dumpEnable && shaderModules.contains(stage.module)) {
          DumpShader<sourceType>(shaderModules[stage.module],
                                 stageToName[stage.stage]);
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
            DumpShader2<sourceType, VkShaderModuleCreateInfo>(
                constModuleInfo, stageToName[pCreateInfos[i].stage.stage]);

          if (loadEnable)
            LoadShader<sourceType, VkShaderModuleCreateInfo>(constModuleInfo);
        }
        next = reinterpret_cast<const VkBaseInStructure *>(next->pNext);
      }

      auto stage = pCreateInfos[i].stage;
      if (dumpEnable && shaderModules.contains(stage.module)) {
        DumpShader<sourceType>(shaderModules[stage.module],
                               stageToName[stage.stage]);
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
    switch (loadType) {
    case LoadSourceType::spirv:
      currentShader = util::LoadSPRV(loadPath);
      break;

    case LoadSourceType::glsl:
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

  template <typename T, typename CreateInfo>
  auto DumpShader2(const CreateInfo *info, const std::string &stageName)
      -> void {
    const auto shader =
        std::vector<T>(reinterpret_cast<const T *>(info->pCode),
                       reinterpret_cast<const T *>(info->pCode) +
                           (info->codeSize / sizeof(T)));
    DumpShader<T>(shader, stageName);
  }

  template <typename T>
  auto DumpShader(const std::vector<T> &shader, const std::string &stageName)
      -> void {
    const auto folder = std::string("/" + stageName + "/");
    const auto hash =
        util::Sha1Hash::compute(shader.data(), shader.size() * sizeof(T))
            .toString();

    if (!std::filesystem::exists(this->dumpPath + folder))
      std::filesystem::create_directory(dumpPath + folder);

    util::SaveSPVToFile<T>(shader, {dumpPath + folder + hash + ".spv"});
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
    if (loadType != LoadSourceType::spirv) {
      std::clog << "[VK_SHADER_GUTS][log]: VK_SHADER_GUTS_LOAD_TYPE = glsl \n";
    }
  }

private:
  bool dumpEnable;
  bool loadEnable;
  std::string dumpPath;
  std::string loadPath;
  std::string loadHash;

  LoadSourceType loadType;

  // Keep the shader until it loads up into the driver.
  std::vector<std::byte> currentShader;

  std::map<VkShaderModule, std::vector<uint32_t>> shaderModules;
  std::map<VkShaderStageFlagBits, std::string> stageToName{
      {VK_SHADER_STAGE_VERTEX_BIT, "VS"},
      {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "TS_Control"},
      {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "TS_evaluation"},
      {VK_SHADER_STAGE_GEOMETRY_BIT, "GS"},
      {VK_SHADER_STAGE_FRAGMENT_BIT, "FS"},
      {VK_SHADER_STAGE_COMPUTE_BIT, "CS"},
  };
  std::map<std::string_view, LoadSourceType> stringToSourceType{
      {"spirv", LoadSourceType::spirv}, {"glsl", LoadSourceType::glsl}};
};

}; // namespace impl