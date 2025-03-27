cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Abalone VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
set(SOURCES
    ./move_generator.cpp
    ./move.cpp
    ./move_direction.cpp
    ./board.cpp
#    ./settings.cpp
    ./main.cpp
)

# Add the header files (optional, for IDEs)
set(HEADERS
    ./move_generator.h
    ./move.h
    ./move_direction.h
    ./board.h
#    ./settings.h
)

# Create the executable
add_executable(abalone ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(abalone PRIVATE ${CMAKE_SOURCE_DIR}/abalone_game)

# Link libraries (if needed)
# target_link_libraries(abalone <library_name>)