cmake_minimum_required(VERSION 3.10)
project(CompilerDesignV2)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Find required libraries (spdlog and fmt)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Add executable
add_executable(driver
    driver.cpp
    lexor.cpp
    parser.cpp
    first_and_follow.cpp
    handler.cpp
    parsing_table.cpp
)

# Link libraries
target_link_libraries(driver PRIVATE spdlog::spdlog fmt::fmt)