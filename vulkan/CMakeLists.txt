set(CORE_SOURCES src/main.cpp src/main.h src/App.cpp src/App.h src/Globals.h)

set(GRAPHICS_SOURCES
    src/Graphics/Window.cpp src/Graphics/Window.h src/Graphics/Scene.cpp
    src/Graphics/Scene.h src/Graphics/Renderer.cpp src/Graphics/Renderer.h)

set(VULKAN_BACKEND_SOURCES
    src/Graphics/vkBackend/Pipeline.cpp
    src/Graphics/vkBackend/Pipeline.h
    src/Graphics/vkBackend/Device.cpp
    src/Graphics/vkBackend/Device.h
    src/Graphics/vkBackend/Swapchain.h
    src/Graphics/vkBackend/Swapchain.cpp
    src/Graphics/vkBackend/DescriptorSetsManager.h
    src/Graphics/vkBackend/DescriptorSetsManager.cpp)

set(MODEL_SOURCES
    src/Graphics/Model/Model.h src/Graphics/Model/Model.cpp
    src/Graphics/Model/Mesh.h src/Graphics/Model/Mesh.cpp
    src/Graphics/Model/ModelLoader.cpp)

set(TEXTURE_SOURCES
    src/Graphics/Texture/Texture.h src/Graphics/Texture/Texture.cpp
    src/Graphics/Texture/TextureManager.h
    src/Graphics/Texture/TextureManager.cpp)

set(CAMERA_SOURCES
    src/Graphics/Cameras/Camera.h src/Graphics/Cameras/Camera.cpp
    src/Graphics/Cameras/Fly.h src/Graphics/Cameras/Fly.cpp
    src/Graphics/Cameras/Orbit.h src/Graphics/Cameras/Orbit.cpp)

add_executable(
  ${PROJECT_NAME} ${CORE_SOURCES} ${GRAPHICS_SOURCES} ${VULKAN_BACKEND_SOURCES}
                  ${MODEL_SOURCES} ${CAMERA_SOURCES} ${TEXTURE_SOURCES})

target_include_directories(${PROJECT_NAME}
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(APP_OUTPUT_DIR "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")

message(
  STATUS "Shader source dir: ${CMAKE_CURRENT_SOURCE_DIR}/src/Assets/Shaders")
message(STATUS "App output dir: ${APP_OUTPUT_DIR}")

# Copy Assets folder into .exe directory
add_custom_target(
  copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/src/Assets" "${APP_OUTPUT_DIR}/Assets"
  COMMENT "Copying assets")

# Copy binary file into .exe directory
add_custom_target(
  copy_assimp_dll ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/include/binaries/assimp-vc143-mt.dll"
    "${APP_OUTPUT_DIR}"
  COMMENT "Copying assimp DLL")
add_dependencies(${PROJECT_NAME} copy_assets copy_assimp_dll)

# Compile shaders
find_program(GLSLC glslc HINTS "$ENV{VULKAN_SDK}/Bin")
file(GLOB SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/Assets/Shaders/*.vert"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/Assets/Shaders/*.frag")

foreach(SHADER ${SHADERS})
  get_filename_component(NAME ${SHADER} NAME)
  add_custom_command(
    OUTPUT "${APP_OUTPUT_DIR}/Assets/Shaders/${NAME}.spv"
    COMMAND ${GLSLC} ${SHADER} -o "${APP_OUTPUT_DIR}/Assets/Shaders/${NAME}.spv"
    DEPENDS ${SHADER} copy_assets
    COMMENT "Compiling Shaders")
  list(APPEND SPV_FILES "${APP_OUTPUT_DIR}/Assets/Shaders/${NAME}.spv")
  message(STATUS "Compiling Shader: ${NAME}")
endforeach()

add_custom_target(Shaders DEPENDS ${SPV_FILES})
add_dependencies(${PROJECT_NAME} Shaders)

# GLFW
add_library(glfw STATIC IMPORTED)
set_target_properties(
  glfw PROPERTIES IMPORTED_LOCATION
                  "${CMAKE_CURRENT_SOURCE_DIR}/include/lib/glfw3.lib")
target_include_directories(glfw INTERFACE include/libraries)
target_link_libraries(glfw INTERFACE opengl32 gdi32 user32 shell32)

# stb
add_library(stb STATIC src/stb.cpp)
target_include_directories(stb PRIVATE include/libraries)

# vulkan
find_package(Vulkan REQUIRED)

# glm
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE include/libraries)

# Assimp
add_library(assimp STATIC IMPORTED)
set_target_properties(
  assimp
  PROPERTIES IMPORTED_LOCATION
             "${CMAKE_CURRENT_SOURCE_DIR}/include/lib/assimp-vc143-mt.lib")
target_include_directories(assimp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# linking
target_link_libraries(${PROJECT_NAME} PRIVATE glfw stb assimp glm
                                              Vulkan::Vulkan)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
