cmake_minimum_required(VERSION 3.16)
project(App C)

set(CMAKE_C_STANDARD 99)

# --- Find raylib headers and libraries ---
find_path(RL_INCLUDE_DIR NAMES raylib.h PATHS /opt/homebrew/opt/raylib/include)
include_directories(${RL_INCLUDE_DIR})

find_library(RL_LIBRARIES NAMES raylib PATHS /opt/homebrew/opt/raylib/lib)

# --- Define project header files ---
file(GLOB_RECURSE HEADER_FILES "src/*.h")

# --- Define project source files ---
file(GLOB_RECURSE SOURCE_FILES "src/*.c")

# --- Define executable ---
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# --- Allow source files in subfolders to include project headers ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --- Link against raylib and system libs ---
target_link_libraries(${PROJECT_NAME} ${RL_LIBRARIES} m pthread)

# --- For preprocessor macros defined by the CLI running CMake file ---
# Debug mode defaulting to OFF
option(DEBUG "Compile with debug mode" OFF)

if(DEBUG)
    message("-- Compiling with DEBUG mode")
    add_definitions(-DDEBUG)
endif(DEBUG)
