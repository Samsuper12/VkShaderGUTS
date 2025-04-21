#pragma once
#include "glslangShaders.hpp"
#include "util.hpp"
#include <any>
#include <chrono>
#include <cstdint>
#include <string_view>
#include <sys/types.h>
#include <thread>

#include "PipelineLibrary.hpp"

namespace impl {

enum class cmd_t {
  playback,
  playstep,
  checkpointType,
  checkpointFunction,
  frameCount
};

struct Command {
  cmd_t cmd;
  std::any payload;
};
class ShaderGuts {
public:
  enum class ShaderLanguage { spirv, glsl };
  enum class CheckpointType { Function };
  enum class CheckpointFunction {
    vkCreateInstance,
    vkCreateDevice,
    vkCreateGraphicsPipelines,
    vkCreateComputePipelines,
    vkCmdBindPipeline,
    vkAcquireNextImageKHR,
    vkQueuePresentKHR
  };

  // TODO: move to the global space later

  struct Playback {
    // TODO:  Pipeline
    CheckpointType checkpointType;
    CheckpointFunction checkpointFunction;
    bool play;
    uint step;
    uint64_t frameCount;
  };

  ShaderGuts(std::string &appname)
      : dumpEnable(false), loadEnable(false), loadLang(ShaderLanguage::spirv),
        appname(appname) {
    namespace fs = std::filesystem;
    playback.play = true;

    bool dump = util::envContainsString("VK_SHADER_GUTS_DUMP_PATH", dumpPath);
    util::envContains<ShaderLanguage>("VK_SHADER_GUTS_DUMP_LANG",
                                      stringToSourceType, dumpLang);

    bool load = util::envContainsString("VK_SHADER_GUTS_LOAD_PATH", loadPath);
    bool hash = util::envContainsString("VK_SHADER_GUTS_LOAD_HASH", loadHash);
    util::envContains<ShaderLanguage>("VK_SHADER_GUTS_LOAD_LANG",
                                      stringToSourceType, loadLang);

    bool pauseFrames = false;
    util::envContainsTrueOrPair(
        "VK_SHADER_GUTS_GUI_PAUSE", pauseFrames,
        [&](std::string l, std::string r) {
          if (l.contains("function")) {
            try {
              auto funcType = functionStringToType.at(r);

              playback.checkpointType = ShaderGuts::CheckpointType::Function;
              playback.checkpointFunction = funcType;
              playback.play = false;
            } catch (std::exception &e) {
              std::clog << "[VK_SHADER_GUTS][GUI][ERR]: bad argument\n ";
            }
          }
        });

    if (dump)
      dumpEnable = fs::exists(dumpPath) ? true : fs::create_directory(dumpPath);

    if (load && hash)
      loadEnable = hash && fs::exists(loadPath);

    if (pauseFrames) {
      playback.play = !pauseFrames;
      playback.checkpointFunction = CheckpointFunction::vkCreateInstance;
      playback.checkpointType = CheckpointType::Function;
    }

    PrintLogs();
  }

  auto LockVulkan(CheckpointFunction f) -> void {
    if (playback.checkpointType == CheckpointType::Function &&
        f == playback.checkpointFunction) {
      while (!playback.play) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (playback.step > 0) {
          playback.step--;
          break;
        }
      }
    }
  }

  auto Execute(const Command &cmd) -> void {

    switch (cmd.cmd) {
    case cmd_t::playback:
      playback.play = std::any_cast<bool>(cmd.payload);
      break;
    case cmd_t::playstep:
      playback.step += std::any_cast<int>(cmd.payload);
      break;
    case cmd_t::checkpointType:
      playback.checkpointType = std::any_cast<CheckpointType>(cmd.payload);
      break;
    case cmd_t::checkpointFunction:
      playback.checkpointFunction =
          std::any_cast<CheckpointFunction>(cmd.payload);
      break;
    default:
      break;
    }
  }

  auto GetFrameCount() const -> uint64_t { return playback.frameCount; }
  auto GetPipeLineLibrary() -> PipelineLibrary & { return pipeLibrary; };

  auto AcquireNextImageKHR() -> void { pipeLibrary.ResetUsed(); };
  auto QueuePresentKHR() -> void {
    playback.frameCount++;
    pipeLibrary.EndOfFrame();
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

  auto
  PreCreateGraphicsPipelines(uint32_t createInfoCount,
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

  auto
  PostCreateGraphicsPiepelines(VkResult result, float duration,
                               uint32_t createInfoCount,
                               const VkGraphicsPipelineCreateInfo *pCreateInfos,
                               VkPipeline *pPipelines) -> void {
    for (size_t i = 0; i < createInfoCount; ++i)
      pipeLibrary.AddPipeline(result, duration, PipelineLibrary::Type::Graphics,
                              createInfoCount, pCreateInfos, pPipelines);
  }

  auto CmdBindPipeline(VkPipeline pipeline) -> void {
    pipeLibrary.MarkUsed(pipeline);
  }

  auto
  PreCreateComputePipelines(uint32_t createInfoCount,
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

  auto PostCreateComputePipelines(
      VkResult result, float duration, uint32_t createInfoCount,
      const VkComputePipelineCreateInfo *pCreateInfos, VkPipeline *pPipelines) {

    for (size_t i = 0; i < createInfoCount; i++) {
      pipeLibrary.AddPipeline(result, duration, PipelineLibrary::Type::Compute,
                              createInfoCount, pCreateInfos, pPipelines);
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
      std::clog << "[VK_SHADER_GUTS][log]: VK_SHADER_GUTS_LOAD_LANG = "
                   "glsl \n";
    }
  }

private:
  bool dumpEnable;
  bool loadEnable;
  std::string dumpPath;
  std::string loadPath;
  std::string loadHash;
  std::string appname;

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

  const std::map<std::string_view, ShaderGuts::CheckpointFunction>
      functionStringToType{
          {"vkCreateInstance",
           ShaderGuts::CheckpointFunction::vkCreateInstance},
          {"vkCreateDevice", ShaderGuts::CheckpointFunction::vkCreateDevice},
          {"vkCreateGraphicsPipelines",
           ShaderGuts::CheckpointFunction::vkCreateGraphicsPipelines},
          {"vkCreateComputePipelines",
           ShaderGuts::CheckpointFunction::vkCreateComputePipelines},
          {
              "vkCmdBindPipeline",
              ShaderGuts::CheckpointFunction::vkCmdBindPipeline,
          },
          {
              "vkAcquireNextImageKHR",
              ShaderGuts::CheckpointFunction::vkAcquireNextImageKHR,
          },
          {
              "vkQueuePresentKHR",
              ShaderGuts::CheckpointFunction::vkQueuePresentKHR,
          },
      };

  Playback playback{};
  PipelineLibrary pipeLibrary;
};

}; // namespace impl