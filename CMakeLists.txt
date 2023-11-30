cmake_minimum_required(VERSION 3.16)
project(opengl-pathtracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    glm GIT_REPOSITORY 
    https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    stb GIT_REPOSITORY 
    https://github.com/nothings/stb
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  tiny-obj
  URL https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/release/tiny_obj_loader.h 
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(tiny-obj)

FetchContent_Declare(
  json
  URL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(json)

file(GLOB EXTERNAL_SOURCE
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

if(WIN32)
    FetchContent_Declare(
        GLEW
        URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip
    )
    FetchContent_MakeAvailable(GLEW)

    FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.4/SDL2-devel-2.26.4-VC.zip
    )
    FetchContent_MakeAvailable(SDL2)

    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/_deps/sdl2-src")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/_deps/glew-src")
endif(WIN32)

project(renderer LANGUAGES CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

include_directories(${stb_SOURCE_DIR})
include_directories(${tiny-obj_SOURCE_DIR})
include_directories(${imgui_SOURCE_DIR})
include_directories(${json_SOURCE_DIR})

add_executable(renderer
    src/main.cpp 
    src/window.cpp src/window.h
    src/renderer.cpp src/renderer.h

    src/gfx/gfx.h
    src/gfx/util.h
    src/gfx/gl.cpp src/gfx/gl.h
    src/gfx/image.cpp src/gfx/image.h

    ${EXTERNAL_SOURCE}
)

target_link_libraries(renderer glm ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

add_custom_target(shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
    COMMENT "copying ${CMAKE_SOURCE_DIR}/shaders to ${CMAKE_BINARY_DIR}/shaders"
)

add_custom_target(assets
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMENT "copying ${CMAKE_SOURCE_DIR}/assets to ${CMAKE_BINARY_DIR}/assets"
)

add_dependencies(renderer shaders assets)

install(TARGETS renderer DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_SOURCE_DIR}/bin)


