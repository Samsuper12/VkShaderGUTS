# Vulkan Shader GUTS layer
A Vulkan layer for dumping and loading shaders.

![.](/docs/first_shader.jpg "Our fellow vkcube has seen better days.")

## How to Build & Install

* `cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=/usr`
* `cmake --build ./build`
* `sudo cmake --install /build`


## ENV vars

* `VK_SHADER_GUTS_ENABLE=1` - Enable the layer
* `VK_SHADER_GUTS_DUMP_PATH=/some/dump/dir` - Sets the directory for dumping shaders.
* `VK_SHADER_GUTS_DUMP_LANG=glsl|spirv` - Set language for out shaders. `spirv` by default.
* `VK_SHADER_GUTS_LOAD_PATH=/some/load/shader.spv` - Specifies the shader file to load
* `VK_SHADER_GUTS_LOAD_HASH=66666666` - Set the hash of the shader you want to replace
* `VK_SHADER_GUTS_LOAD_LANG=glsl|spirv` - Set language of the source file. `spirv` by default.

## Examples of usage

### Dumping shaders:
```sh
export VK_SHADER_GUTS_ENABLE=1 
export VK_SHADER_GUTS_DUMP_PATH=$HOME/Documents/dump/

wine ~/renderdoccmd.exe replay ~/hitman2_2025.03.20_17.58_frame12740.rdc
```
### Loading a shader
```sh
export VK_SHADER_GUTS_ENABLE=1 
export VK_SHADER_GUTS_LOAD_PATH=$HOME/frag.spv
export VK_SHADER_GUTS_LOAD_HASH=3077582152445e6cddc7a384774b97486e1bc718

vkcube
```
