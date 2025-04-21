cmake_minimum_required(VERSION 3.30)

project(VulkanShaderGuts 
	LANGUAGES CXX C
	VERSION 0.0.1)

include(GNUInstallDirs)

find_package(VulkanHeaders CONFIG)
find_package(Vulkan CONFIG COMPONENTS glslang glslangValidator glslc)
find_package(glslang CONFIG)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)

find_package(spirv_cross_c_shared)
if (spirv_cross_c_shared_FOUND)
        message(STATUS "Found SPIRV-Cross C API!")
else()
        message(STATUS "Could not find SPIRV-Cross C API!")
endif()

set(LAYER_JSON "VkLayer_shader_guts")

if(APPLE)
    set(LAYER_INSTALL_DIR "/Library/Application Support/Vulkan/ImplicitLayers")
else() # Linux
    set(LAYER_INSTALL_DIR "/etc/vulkan/implicit_layer.d")
endif()


# Layer
add_library(guts_layer SHARED)

target_include_directories(guts_layer
PRIVATE
	src/
)

target_sources(guts_layer
PUBLIC 
	src/layer.cpp
	src/sha1.c
	src/sha1_util.cpp
)

target_link_libraries(guts_layer
PRIVATE
    Vulkan::Headers

	glslang::glslang
	glslang::glslang-default-resource-limits
	glslang::SPIRV
	glslang::SPVRemapper

	spirv-cross-c-shared
	spirv-cross-core
	spirv-cross-glsl
	spirv-cross-reflect
)

set_target_properties(guts_layer PROPERTIES
    OUTPUT_NAME "${LAYER_JSON}"
	CXX_STANDARD 23
	CXX_EXTENSIONS YES
	CMAKE_CXX_SCAN_FOR_MODULES ON
)

configure_file(${CMAKE_SOURCE_DIR}/${LAYER_JSON}.temp.json ${CMAKE_BINARY_DIR}/${LAYER_JSON}.json @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/${LAYER_JSON}.json DESTINATION ${LAYER_INSTALL_DIR})
install(TARGETS guts_layer
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


# Gui

add_executable(guts_gui)

target_sources(guts_gui PRIVATE 
src/gui/gui.cpp
src/gui/backend/imgui_impl_glfw.cpp
src/gui/backend/imgui_impl_opengl3.cpp

src/gui/menu/PipelinesMenu.cpp
src/gui/menu/PlaybackMenu.cpp
)

target_link_libraries(guts_gui PRIVATE
Vulkan::Headers

glbinding::glbinding
glbinding::glbinding-aux

glfw

imgui::imgui
)

target_include_directories(guts_gui
PRIVATE
	src/
	src/gui/
	src/gui/menu
)

set_target_properties(guts_gui PROPERTIES
    OUTPUT_NAME "VkShaderGUTS_GUI"
	CXX_STANDARD 23
	CXX_EXTENSIONS YES
	#MACOSX_BUNDLE YES
)

install(TARGETS guts_layer
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})