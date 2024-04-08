cmake_minimum_required(VERSION 3.27)
project(XO)

# Specify the C++ compiler (clang++)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_COMPILER "/usr/bin/clang")

set(CMAKE_CXX_STANDARD 23)

# Link the standard C++ library
set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++")

# Specify the root directory where header files are located
set(HEADER_ROOT ${CMAKE_SOURCE_DIR}/lib)

# Recursively search for header files in all subdirectories of the lib directory
file(GLOB_RECURSE HEADER_FILES ${HEADER_ROOT}/*.h ${HEADER_ROOT}/*.hpp)
foreach(FILE_PATH ${HEADER_FILES})
    get_filename_component(DIR_PATH ${FILE_PATH} DIRECTORY)
    include_directories(${DIR_PATH})
endforeach()

# Specify the paths to the conan-generated configuration files
set(nlohmann_json_DIR ${CMAKE_BINARY_DIR})
set(spdlog_DIR ${CMAKE_BINARY_DIR})
set(fmt_DIR ${CMAKE_BINARY_DIR})

# Find the packages managed by conan
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Add an executable for the server
file(GLOB_RECURSE SERVER_SOURCES src/main.cxx lib/epoll_server/*.cxx lib/game_session/*.cxx lib/game_state/*.cxx lib/shared_mem/*.cxx)
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server nlohmann_json::nlohmann_json spdlog::spdlog fmt::fmt)

# Add an executable for the client
file(GLOB_RECURSE CLIENT_SOURCES lib/client/*.cxx lib/game_session/*.cxx lib/game_state/*.cxx lib/shared_mem/*.cxx)
add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client nlohmann_json::nlohmann_json spdlog::spdlog fmt::fmt)