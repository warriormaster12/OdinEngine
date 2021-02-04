cmake_minimum_required(VERSION 3.0.0)
project(VulkanEngine.x86_64 VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)


file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/Engine/Source/*)

add_executable(VulkanEngine.x86_64 ${SRC})

target_include_directories(VulkanEngine.x86_64 PUBLIC
    Engine/Source/
)



if (UNIX AND NOT APPLE)
    set (LINUX TRUE)
	target_compile_definitions(VulkanEngine.x86_64 PRIVATE -DVK_USE_PLATFORM_XLIB_KHR)
	target_include_directories(VulkanEngine.x86_64 PRIVATE Vulkan::Vulkan)
	target_include_directories(VulkanEngine.x86_64 PRIVATE ${X11+INCLUDE_DIR})
	target_link_libraries(VulkanEngine.x86_64 PRIVATE Vulkan::Vulkan)
	target_link_libraries(VulkanEngine.x86_64 PRIVATE ${X11_LIBRARIES} -lm)
endif ()

if (WIN32)
	target_compile_definitions(VulkanEngine.x86_64 PRIVATE -DVK_USE_PLATFORM_WIN32LIB_KHR)
	target_include_directories(VulkanEngine.x86_64 PRIVATE Vulkan::Vulkan)
	target_include_directories(VulkanEngine.x86_64 PRIVATE ${WIN32+INCLUDE_DIR})
	target_link_libraries(VulkanEngine.x86_64 PRIVATE Vulkan::Vulkan)
	target_link_libraries(VulkanEngine.x86_64 PRIVATE ${WIN32_LIBRARIES})   
endif ()


#glfw
add_subdirectory(Third-Party/glfw EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine.x86_64 PRIVATE glfw) 

#vk-bootstrap
add_subdirectory(Third-Party/vk-bootstrap EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine.x86_64 PUBLIC vk-bootstrap -ldl)


#glm
add_subdirectory(Third-Party/glm EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine.x86_64 PRIVATE glm)

#glslang 
add_subdirectory(Third-Party/glslang EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine.x86_64 PRIVATE glslang SPIRV)

#spdlog
add_subdirectory(Third-Party/spdlog EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine.x86_64 PRIVATE spdlog)



file(COPY ${PROJECT_SOURCE_DIR}/.Shaders DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/EngineAssets DESTINATION ${PROJECT_BINARY_DIR})


