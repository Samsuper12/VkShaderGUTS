#pragma once
#include "util.hpp"
#include <cstdint>
#include <expected>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <regex>

namespace util::shaders {

const std::map<std::string_view, EShLanguage> shaderTypeESh{
    {".vert", EShLangVertex},
    {".frag", EShLangFragment},
    {".comp", EShLangCompute},
    {".geom", EShLangGeometry},
};

struct GLSLCompileParams {
  std::string shaderSource;
  size_t shaderVersion;
  EShLanguage shaderStage;
};

inline auto findGLSLVersion(const std::string &shader)
    -> std::expected<uint, std::string> {
  std::smatch match;
  auto reg = std::regex(R"(#version\s+(\d+))");
  if (std::regex_search(shader, match, reg)) {
    return std::stoi(match[1]);
  }
  return std::unexpected(std::format(
      "[VK_SHADER_GUTS][err]: Can't recognize glsl shader version."));
}

inline auto findShaderType(const std::filesystem::path path)
    -> std::expected<EShLanguage, std::string> {

  if (shaderTypeESh.contains(path.extension().c_str())) {
    return shaderTypeESh.at(path.extension().c_str());
  }

  return std::unexpected(std::format(
      "[VK_SHADER_GUTS][err]: Can't recognize file extension for file: {}\n",
      path.c_str()));
}

inline auto compileGLSL(const GLSLCompileParams &params)
    -> std::vector<std::byte> {
  glslang::InitializeProcess();
  const char *shaderStrings[1];

  shaderStrings[0] = params.shaderSource.data();
  glslang::TShader shader(params.shaderStage);
  shader.setStrings(shaderStrings, 1);

  EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

  if (!shader.parse(GetDefaultResources(), params.shaderVersion, true,
                    messages)) {
    std::clog << "[VK_SHADER_GUTS][err]: " << shader.getInfoLog() << " "
              << shader.getInfoDebugLog() << "\n";
    return {};
  }

  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(messages)) {
    std::clog << "[VK_SHADER_GUTS][err]: " << program.getInfoLog() << " "
              << program.getInfoDebugLog() << "\n";
    return {};
  }

  glslang::TIntermediate *intermediate =
      program.getIntermediate(params.shaderStage);

  if (!intermediate) {
    std::clog << "[VK_SHADER_GUTS][err]: Failed to get SPIRV intermediate of "
                 "the shader.\n";
    return {};
  }

  std::vector<uint32_t> spirvOutput;
  glslang::GlslangToSpv(*intermediate, spirvOutput);

  glslang::FinalizeProcess();

  return std::vector<std::byte>(
      reinterpret_cast<std::byte *>(spirvOutput.data()),
      reinterpret_cast<std::byte *>(spirvOutput.data()) +
          (spirvOutput.size() * sizeof(uint32_t)));
}
} // namespace util::shaders
