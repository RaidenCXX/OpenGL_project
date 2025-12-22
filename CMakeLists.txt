


cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



project(lernOpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(${PROJECT_NAME}
  source/main.cpp
  source/shader.cpp
  source/stb_image.cpp
  source/camera.cpp
  source/system.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)


cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  ${CMAKE_BINARY_DIR}/deps/libglfw.so
  ${CMAKE_BINARY_DIR}/deps/libGLEW.so
  ${CMAKE_BINARY_DIR}/deps/libassimp.so
  OpenGL::GL
  dl
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  BUILD_RPATH "$ORIGIN/deps"
  INSTALL_RPATH "$ORIGIN/deps"
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/deps)

file(GLOB GLEW_LIBS
  "${PROJECT_SOURCE_DIR}/deps/glew/libGLEW.so*"
)

file(GLOB GLFW_LIBS
  "${PROJECT_SOURCE_DIR}/deps/glfw/libglfw.so*"
)

file(GLOB ASSIMP_LIBS
  "${PROJECT_SOURCE_DIR}/deps/assimp/libassimp.so*"
)

file(COPY
  ${GLEW_LIBS}
  ${GLFW_LIBS}
  ${ASSIMP_LIBS}
  DESTINATION ${CMAKE_BINARY_DIR}/deps
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/shader

  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/source/shader/
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/shader/
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets

  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/assets
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
