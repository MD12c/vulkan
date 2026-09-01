add_executable(${PROJECT_NAME} src/main.cpp src/main.h src/Graphics/Window.cpp
                               src/Graphics/Window.h src/Globals.h)

message(
  STATUS "Shader source dir: ${CMAKE_CURRENT_SOURCE_DIR}/src/Assets/Shaders")

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/Assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets")

add_library(glfw STATIC IMPORTED)
set_target_properties(
  glfw PROPERTIES IMPORTED_LOCATION
                  "${CMAKE_CURRENT_SOURCE_DIR}/include/lib/glfw3.lib")
target_include_directories(glfw INTERFACE include/libraries)
target_link_libraries(glfw INTERFACE opengl32 gdi32 user32 shell32)

add_library(stb STATIC src/stb.cpp)
target_include_directories(stb PRIVATE include/libraries)

find_package(Vulkan REQUIRED)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE include/libraries)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw stb glm Vulkan::Vulkan)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
