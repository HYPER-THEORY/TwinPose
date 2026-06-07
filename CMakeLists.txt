cmake_minimum_required(VERSION 3.20)
project(TwinPose LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/intermediate)

foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/binaries/${CONFIG_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/binaries/${CONFIG_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/intermediate/${CONFIG_TYPE})
endforeach()

set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries)
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/binaries)
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_PDB_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/binaries/${CONFIG_TYPE})
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/binaries/${CONFIG_TYPE})
endforeach()

function(set_intermediate_directory TARGET_NAME)
    if(MSVC)
        set_target_properties(${TARGET_NAME} PROPERTIES
            VS_INTERMEDIATE_DIRECTORY "${CMAKE_SOURCE_DIR}/intermediate/${TARGET_NAME}/$<CONFIG>/"
            PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/$<CONFIG>"
            COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/binaries/$<CONFIG>"
        )
        set_property(TARGET ${TARGET_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        )
    endif()
endfunction()

# ==============================================================================
# SDL2
# ==============================================================================
set(SDL_SHARED            ON  CACHE BOOL "Build SDL2 as shared library")
set(SDL_STATIC            OFF CACHE BOOL "Build SDL2 as static library")
set(SDL_TEST              OFF CACHE BOOL "Build SDL2 test programs")
set(SDL2_DISABLE_INSTALL   ON CACHE BOOL "Disable SDL2 install targets")
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable SDL2 uninstall targets")
add_subdirectory(libraries/sdl EXCLUDE_FROM_ALL)

# ==============================================================================
# OpenCV
# ==============================================================================
set(BUILD_SHARED_LIBS                       ON  CACHE BOOL "Build OpenCV shared libraries")
set(BUILD_opencv_world                      ON  CACHE BOOL "Build single opencv_world library")
set(BUILD_TESTS                             OFF CACHE BOOL "Disable OpenCV tests")
set(BUILD_PERF_TESTS                        OFF CACHE BOOL "Disable OpenCV performance tests")
set(BUILD_EXAMPLES                          OFF CACHE BOOL "Disable OpenCV examples")
set(BUILD_DOCS                              OFF CACHE BOOL "Disable OpenCV docs")
set(BUILD_opencv_apps                       OFF CACHE BOOL "Disable OpenCV apps")
set(BUILD_opencv_python_bindings_generator  OFF CACHE BOOL "Disable OpenCV python bindings")
set(BUILD_opencv_python_tests               OFF CACHE BOOL "Disable OpenCV python tests")
set(BUILD_opencv_java_bindings_generator    OFF CACHE BOOL "Disable OpenCV java bindings")
set(BUILD_opencv_js                         OFF CACHE BOOL "Disable OpenCV js")
set(BUILD_JAVA                              OFF CACHE BOOL "Disable OpenCV Java")
set(BUILD_PACKAGE                           OFF CACHE BOOL "Disable OpenCV package")
set(WITH_OPENCL                             OFF CACHE BOOL "Disable OpenCL")
set(WITH_CUDA                               OFF CACHE BOOL "Disable CUDA")
set(WITH_PROTOBUF                           OFF CACHE BOOL "Disable Protobuf")
set(WITH_IPP                                OFF CACHE BOOL "Disable Intel IPP")
set(WITH_IPP_IW                             OFF CACHE BOOL "Disable Intel IPP IW")
set(OPENCV_IPP_GAUSSIAN_BLUR                OFF CACHE BOOL "Disable IPP Gaussian Blur")
set(OPENCV_IPP_MEAN                         OFF CACHE BOOL "Disable IPP Mean")
set(OPENCV_IPP_MINMAX                       OFF CACHE BOOL "Disable IPP MinMax")
set(OPENCV_IPP_SUM                          OFF CACHE BOOL "Disable IPP Sum")
set(OPENCV_GENERATE_SETUPVARS               OFF CACHE BOOL "Disable OpenCV setup vars")
add_subdirectory(libraries/opencv EXCLUDE_FROM_ALL)

set(OPENCV_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/libraries/opencv/include
    ${CMAKE_BINARY_DIR}
)
file(GLOB OPENCV_MODULE_DIRS "${CMAKE_SOURCE_DIR}/libraries/opencv/modules/*")
foreach(OPENCV_MODULE_DIR ${OPENCV_MODULE_DIRS})
    if(IS_DIRECTORY ${OPENCV_MODULE_DIR}/include)
        list(APPEND OPENCV_INCLUDE_DIRS ${OPENCV_MODULE_DIR}/include)
    endif()
endforeach()

# ==============================================================================
# Ink3D
# ==============================================================================
file(GLOB_RECURSE INK_HEADERS "libraries/ink/*.h")
file(GLOB_RECURSE INK_SOURCES "libraries/ink/*.cpp" "libraries/ink/*.c")

add_library(ink SHARED ${INK_HEADERS} ${INK_SOURCES})
target_include_directories(ink PUBLIC ${CMAKE_SOURCE_DIR}/libraries)
target_include_directories(ink PUBLIC ${CMAKE_SOURCE_DIR}/libraries/ink)
set_intermediate_directory(ink)

set_target_properties(ink PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(WIN32)
    target_link_libraries(ink PUBLIC opengl32)
endif()

# ==============================================================================
# nlohmann json
# ==============================================================================
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_SOURCE_DIR}/libraries/json/single_include
)

# ==============================================================================
# Dear ImGui
# ==============================================================================
add_library(imgui STATIC
    ${CMAKE_SOURCE_DIR}/libraries/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/libraries/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/libraries/imgui/imgui_spectrum.cpp
    ${CMAKE_SOURCE_DIR}/libraries/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/libraries/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/libraries/imgui/backends/imgui_impl_sdl2.cpp
    ${CMAKE_SOURCE_DIR}/libraries/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/libraries/imgui
    ${CMAKE_SOURCE_DIR}/libraries/imgui/backends
)
target_link_libraries(imgui PUBLIC SDL2::SDL2)
set_intermediate_directory(imgui)

# ==============================================================================
# TwinPose
# ==============================================================================
file(GLOB_RECURSE TWINPOSE_HEADERS "src/*.h")
file(GLOB_RECURSE TWINPOSE_SOURCES "src/*.cpp")

add_executable(TwinPose ${TWINPOSE_HEADERS} ${TWINPOSE_SOURCES})

target_include_directories(TwinPose PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libraries
    ${CMAKE_SOURCE_DIR}/libraries/json/single_include
    ${OPENCV_INCLUDE_DIRS}
)

set_intermediate_directory(TwinPose)

target_link_libraries(TwinPose PRIVATE
    ink
    nlohmann_json
    imgui
    SDL2::SDL2main
    SDL2::SDL2
    opencv_world
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source Files" FILES ${TWINPOSE_SOURCES})
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Header Files" FILES ${TWINPOSE_HEADERS})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TwinPose)

set_target_properties(TwinPose PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(TARGET SDL2)
    set_target_properties(SDL2 PROPERTIES FOLDER "Libraries/SDL2")
endif()
if(TARGET SDL2main)
    set_target_properties(SDL2main PROPERTIES FOLDER "Libraries/SDL2")
endif()
if(TARGET sdl_headers_copy)
    set_target_properties(sdl_headers_copy PROPERTIES FOLDER "Libraries/SDL2")
endif()

if(TARGET opencv_world)
    set_target_properties(opencv_world PROPERTIES FOLDER "Libraries/OpenCV")
endif()

if(TARGET ink)
    set_target_properties(ink PROPERTIES FOLDER "Libraries/Ink")
endif()

if(TARGET imgui)
    set_target_properties(imgui PROPERTIES FOLDER "Libraries/ImGui")
endif()
