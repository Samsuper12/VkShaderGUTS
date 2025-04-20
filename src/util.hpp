#pragma once

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace util {

inline auto SaveGLSLToFile(const std::vector<std::byte> &shader,
                           std::filesystem::path path) -> void {
  // need to convert into v<uint32_t>
  spirv_cross::CompilerGLSL compiler(
      std::vector<uint32_t>(reinterpret_cast<const uint32_t *>(shader.data()),
                            reinterpret_cast<const uint32_t *>(shader.data()) +
                                (shader.size() / sizeof(uint32_t))));
  spirv_cross::ShaderResources resources = compiler.get_shader_resources();

  spirv_cross::CompilerGLSL::Options options;
  options.vulkan_semantics = true;
  compiler.set_common_options(options);

  std::string glslCode = compiler.compile();

  if (std::ofstream file(path, std::ios::binary); file.is_open()) {
    file.write(glslCode.c_str(), glslCode.size());
    file.close();
  }
}

inline auto getEnv(std::string_view env) -> std::optional<std::string> {
  if (auto value = std::getenv(env.data()))
    return std::string(value);
  return std::nullopt;
}

template <typename T>
auto envContains(std::string_view var, std::map<std::string_view, T> matches,
                 T &setOnMatch) -> bool {
  auto env = util::getEnv(var);

  if (env && matches.contains(env.value())) {
    setOnMatch = matches[env.value()];
    return true;
  }
  return false;
}

inline auto StringContainsTrue(std::string_view str) -> bool {
  return (str.contains("1") || str.contains("true") || str.contains("True") ||
          str.contains("TRUE"));
}

inline auto envContainsTrue(std::string_view var, bool &setOnMatch) -> void {
  auto env = util::getEnv(var);

  setOnMatch = env && StringContainsTrue(env.value());
}

inline auto envContainsTrueOrPair(
    std::string_view var, bool &onBoolMatch,
    const std::function<void(std::string, std::string)> &onStringMatch)
    -> void {
  auto env = util::getEnv(var);

  if (!env.has_value())
    return;

  if (StringContainsTrue(env.value()))
    onBoolMatch = true;

  if (env.value().size() < 3 || env.value().find(':') == std::string::npos)
    return;

  auto ret = env.value();
  onStringMatch(ret.substr(0, ret.find(':')), ret.substr(ret.find(':') + 1));
}

inline auto
envContainsPair(std::string_view var,
                const std::function<void(std::string, std::string)> &str)
    -> void {
  auto env = util::getEnv(var);

  if (!env.has_value() || env.value().size() < 3 ||
      env.value().find(':') == std::string::npos)
    return;

  auto ret = env.value();
  str(ret.substr(0, ret.find(':')), ret.substr(ret.find(':') + 1));
}

inline auto envContainsString(std::string_view var, std::string &str) -> bool {
  auto env = util::getEnv(var);
  if (env)
    str = env.value();
  return env != std::nullopt;
}

// Thanks to the DXVK project for this util class. I'm a little bit confused
// why there is no optimal (size/functions) package in vcpkg for sha1.
// https://github.com/doitsujin/dxvk

class Sha1Hash {
public:
  using Sha1Digest = std::array<uint8_t, 20>;

  struct Sha1Data {
    const void *data;
    size_t size;
  };

  Sha1Hash() {}
  Sha1Hash(const Sha1Digest &digest) : m_digest(digest) {}

  std::string toString() const;

  uint32_t dword(uint32_t id) const {
    return uint32_t(m_digest[4 * id + 0]) << 0 |
           uint32_t(m_digest[4 * id + 1]) << 8 |
           uint32_t(m_digest[4 * id + 2]) << 16 |
           uint32_t(m_digest[4 * id + 3]) << 24;
  }

  bool operator==(const Sha1Hash &other) const {
    return !std::memcmp(this->m_digest.data(), other.m_digest.data(),
                        other.m_digest.size());
  }

  bool operator!=(const Sha1Hash &other) const {
    return !this->operator==(other);
  }

  static Sha1Hash compute(const void *data, size_t size);

  static Sha1Hash compute(size_t numChunks, const Sha1Data *chunks);

  template <typename T> static Sha1Hash compute(const T &data) {
    return compute(&data, sizeof(T));
  }

private:
  Sha1Digest m_digest;
};

template <typename T>
inline void SaveSPVToFile(const std::vector<T> &code,
                          const std::string &filename) {
  if (std::ofstream file(filename, std::ios::binary); file.is_open()) {
    file.write(reinterpret_cast<const char *>(code.data()),
               code.size() * sizeof(T));
    file.close();
  }
}

inline auto LoadFile(std::filesystem::path path) -> std::string {
  if (path.empty() || !std::filesystem::exists(path)) {
    std::cerr << "[VK_SHADER_GUTS][err]: Can't find file: " << path << "\n";
    return {};
  }

  if (std::ifstream file(path, std::ios::binary); file.is_open()) {
    return std::string(std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>());
  }

  return {};
}

inline auto LoadSPRV(std::filesystem::path path) -> std::vector<std::byte> {
  if (path.empty() || !std::filesystem::exists(path)) {
    std::clog << "[VK_SHADER_GUTS][err]: Can't find shader to load: " << path
              << "\n";
    return {};
  }

  std::vector<std::byte> ret;

  if (std::ifstream file(path, std::ios::ate | std::ios::binary);
      file.is_open()) {
    size_t fileSize = static_cast<size_t>(file.tellg());
    ret.resize(fileSize / sizeof(std::byte));
    file.seekg(0);
    file.read((char *)ret.data(), fileSize);
    file.close();
  }
  return ret;
}

using Sha1Digest = std::array<uint8_t, 20>;

struct Sha1Data {
  const void *data;
  size_t size;
};

} // namespace util