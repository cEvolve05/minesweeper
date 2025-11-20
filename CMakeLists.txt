cmake_minimum_required(VERSION 3.21)
project(minesweeper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

# packages
find_package(spdlog CONFIG REQUIRED)
find_package(Slint QUIET)
if (NOT Slint_FOUND)
  message("Slint could not be located in the CMake module search path. Downloading it from Git and building it locally")
  include(FetchContent)
  FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    # hard code here, ok for small project
    GIT_TAG release/1.14
    SOURCE_SUBDIR api/cpp
  )
  FetchContent_MakeAvailable(Slint)
endif (NOT Slint_FOUND)

file(GLOB_RECURSE CPP_SOURCE CONFIGURE_DEPENDS src/*.cpp src/*.h src/*.hpp)
file(GLOB_RECURSE SLINT_SOURCE CONFIGURE_DEPENDS ui/*.slint)

add_executable(minesweeper ${CPP_SOURCE})
slint_target_sources(minesweeper ${SLINT_SOURCE} NAMESPACE ui)

target_link_libraries(minesweeper 
  PRIVATE
    Slint::Slint
    spdlog::spdlog
)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
    add_custom_command(TARGET minesweeper POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:minesweeper> $<TARGET_FILE_DIR:minesweeper> COMMAND_EXPAND_LISTS)
endif()