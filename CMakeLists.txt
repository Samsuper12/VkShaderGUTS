cmake_minimum_required(VERSION 3.30)

project(VulkanShaderGuts 
	LANGUAGES CXX C
	VERSION 0.0.1)

include(GNUInstallDirs)

find_package(VulkanHeaders CONFIG)
find_package(Vulkan CONFIG COMPONENTS glslang glslangValidator glslc)
find_package(glslang CONFIG)

set(LAYER_JSON "VkLayer_shader_guts")

if(APPLE)
    set(LAYER_INSTALL_DIR "/Library/Application Support/Vulkan/ImplicitLayers")
else() # Linux
    set(LAYER_INSTALL_DIR "/etc/vulkan/implicit_layer.d")
endif()

add_library(${CMAKE_PROJECT_NAME} SHARED)

target_include_directories(${CMAKE_PROJECT_NAME}
PRIVATE
	src/
)

target_sources(${CMAKE_PROJECT_NAME}
PUBLIC 
	src/layer.cpp
	src/sha1.c
	src/sha1_util.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
PRIVATE
    Vulkan::Headers
	glslang::glslang
	glslang::glslang-default-resource-limits
	glslang::SPIRV
	glslang::SPVRemapper

)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${LAYER_JSON}"
	CXX_STANDARD 23
	CXX_EXTENSIONS YES
	CMAKE_CXX_SCAN_FOR_MODULES ON
)

configure_file(${CMAKE_SOURCE_DIR}/${LAYER_JSON}.temp.json ${CMAKE_BINARY_DIR}/${LAYER_JSON}.json @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/${LAYER_JSON}.json DESTINATION ${LAYER_INSTALL_DIR})
install(TARGETS ${CMAKE_PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
