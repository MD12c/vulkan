add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/main.h
  src/App.cpp
  src/App.h
  src/Graphics/Window.cpp
  src/Graphics/Window.h
  src/Graphics/Pipeline.cpp
  src/Graphics/Pipeline.h
  src/Graphics/Device.cpp
  src/Graphics/Device.h
  src/Globals.h
  src/Graphics/Swapchain.h
  src/Graphics/Swapchain.cpp
  src/Graphics/Model.h
  src/Graphics/Model.cpp
  src/Graphics/Renderer.h
  src/Graphics/Renderer.cpp
  src/Graphics/Scene.h
  src/Graphics/Scene.cpp)

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

# linking
target_link_libraries(${PROJECT_NAME} PRIVATE glfw stb glm Vulkan::Vulkan)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
