#pragma once
#include "util.hpp"
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <map>

namespace util::shaders {

const std::map<std::string_view, EShLanguage> shaderTypeESh{
    {".vert", EShLangVertex},
    {".frag", EShLangFragment},
    {".comp", EShLangCompute},
    {".geom", EShLangGeometry},
};

const std::map<std::string_view, VkShaderStageFlagBits> shaderTypeVk{
    {".vert", VK_SHADER_STAGE_VERTEX_BIT},
    {".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
    {".comp", VK_SHADER_STAGE_COMPUTE_BIT},
    {".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
};

struct GLSLCompileParams {
  std::string shaderCode;
  size_t shaderVersion;
  EShLanguage shaderStage;
};

inline auto compileShader(const GLSLCompileParams &params,
                          std::vector<uint32_t> &output) -> bool {
  glslang::InitializeProcess();
  const char *shaderStrings[1];

  shaderStrings[0] = params.shaderCode.data();

  glslang::TShader shader(params.shaderStage);
  shader.setStrings(shaderStrings, 1);

  EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

  if (!shader.parse(GetDefaultResources(), params.shaderVersion, true,
                    messages)) {
    std::clog << "[VK_SHADER_GUTS][err]: " << shader.getInfoLog() << " "
              << shader.getInfoDebugLog() << "\n";
    return false;
  }

  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(messages)) {
    std::clog << "[VK_SHADER_GUTS][err]: " << program.getInfoLog() << " "
              << program.getInfoDebugLog() << "\n";
    return false;
  }

  glslang::GlslangToSpv(*program.getIntermediate(params.shaderStage), output);
  glslang::FinalizeProcess();
  return true;
}
} // namespace util::shaders
