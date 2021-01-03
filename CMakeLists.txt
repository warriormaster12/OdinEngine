cmake_minimum_required(VERSION 3.0.0)
project(VulkanEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)


file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/Engine/Source/*)

add_executable(VulkanEngine ${SRC})

target_include_directories(VulkanEngine PUBLIC
    Engine/Source/
)



if (UNIX AND NOT APPLE)
    set (LINUX TRUE)
	target_compile_definitions(VulkanEngine PRIVATE -DVK_USE_PLATFORM_XLIB_KHR)
	target_include_directories(VulkanEngine PRIVATE Vulkan::Vulkan)
	target_include_directories(VulkanEngine PRIVATE ${X11+INCLUDE_DIR})
	target_link_libraries(VulkanEngine PRIVATE Vulkan::Vulkan)
	target_link_libraries(VulkanEngine PRIVATE ${X11_LIBRARIES} -lm)
endif ()

if (WIN32)
	target_compile_definitions(VulkanEngine PRIVATE -DVK_USE_PLATFORM_WIN32LIB_KHR)
	target_include_directories(VulkanEngine PRIVATE Vulkan::Vulkan)
	target_include_directories(VulkanEngine PRIVATE ${WIN32+INCLUDE_DIR})
	target_link_libraries(VulkanEngine PRIVATE Vulkan::Vulkan)
	target_link_libraries(VulkanEngine PRIVATE ${WIN32_LIBRARIES})   
endif ()

#SDL2
add_subdirectory(third-party/SDL2-2.0.14 EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine PRIVATE SDL2)

#vk-bootstrap
add_subdirectory(third-party/vk-bootstrap EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine PUBLIC vk-bootstrap -ldl)


#glm
add_subdirectory(third-party/glm EXCLUDE_FROM_ALL)
target_link_libraries(VulkanEngine PRIVATE glm)

#glslang 
#add_subdirectory(lib/glslang EXCLUDE_FROM_ALL)
#target_link_libraries(VulkanEngine PRIVATE glslang SPIRV)

file(COPY ${PROJECT_SOURCE_DIR}/.Shaders DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/EngineAssets DESTINATION ${PROJECT_BINARY_DIR})


