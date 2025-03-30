#pragma once

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace util {

// Thanks to the DXVK project for this util class. I'm a little bit confused why
// there is no optimal (size/functions) package in vcpkg for sha1.
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

inline auto LoadSPVtoVector(std::filesystem::path path)
    -> std::vector<std::byte> {
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