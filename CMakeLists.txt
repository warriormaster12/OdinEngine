cmake_minimum_required(VERSION 3.0.0)
project(OdinEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/Engine/Source/*)

add_executable(OdinEngine ${SRC})

target_include_directories(OdinEngine PUBLIC
    Engine/Source/Asset_Manager/Include/
    Engine/Source/Camera/Include/
    Engine/Source/Editor/Include/
    Engine/Source/Helpers/Logger/Include/
    Engine/Source/Helpers/Queuer/
    Engine/Source/Helpers/Unordered_Finder/
    Engine/Source/Helpers/Statistics/Include/
    Engine/Source/RendererBackends/Vulkan/Include/
    Engine/Source/RendererBackends/Vulkan/vk_components/Include
    Engine/Source/Main/Include/
    Engine/Source/Window/Include/
    Engine/Source/Renderer/Components/Include/
    Engine/Source/Renderer/Pipelines/Include/
    Engine/Source/Editor/Include/
    Engine/Source/SceneManager/Include/
    Engine/Source/SceneManager/Components/Include/
    Engine/Source/IO/Include/
    
    Engine/Source/Third-Party/catch2/
    Engine/Source/Third-Party/imgui/Include/
    Engine/Source/Third-Party/lz4/Include/
    Engine/Source/Third-Party/nlohmann_json/
    Engine/Source/Third-Party/stb_image/
    Engine/Source/Third-Party/tinyobjloader/Include/
    Engine/Source/Third-Party/Vma/Include/
    Engine/Source/Third-Party/spirv-reflect/Include/
    Engine/Source/Third-Party/vk-bootstrap/Include/
)



if (UNIX AND NOT APPLE)
    set (LINUX TRUE)
    if(DGLFW_USE_WAYLAND=ON)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR)
    else()
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
    endif ()
endif ()

if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
	target_include_directories(OdinEngine PRIVATE ${WIN32+INCLUDE_DIR})
	target_link_libraries(OdinEngine PRIVATE ${WIN32_LIBRARIES})   
endif ()


#glfw
add_subdirectory(Third-Party/glfw EXCLUDE_FROM_ALL)
target_link_libraries(OdinEngine PRIVATE glfw) 

#volk
add_subdirectory(Third-Party/volk EXCLUDE_FROM_ALL)
target_link_libraries(OdinEngine PRIVATE volk) 

#glm
add_subdirectory(Third-Party/glm EXCLUDE_FROM_ALL)
target_link_libraries(OdinEngine PRIVATE glm)

#glslang 
add_subdirectory(Third-Party/glslang EXCLUDE_FROM_ALL)
target_link_libraries(OdinEngine PRIVATE glslang SPIRV)

#spdlog
add_subdirectory(Third-Party/spdlog EXCLUDE_FROM_ALL)
target_link_libraries(OdinEngine PRIVATE spdlog)

add_subdirectory(Engine/Test)


file(COPY ${PROJECT_SOURCE_DIR}/EngineAssets DESTINATION ${PROJECT_BINARY_DIR})


