cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Abalone VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
set(SOURCES
        src/move_generator.cpp
        src/move.cpp
        src/move_direction.cpp
        src/board.cpp
#    ./settings.cpp
        src/main.cpp
)

# Add the header files (optional, for IDEs)
set(HEADERS
        include/move_generator.h
        include/move.h
        include/move_direction.h
        include/board.h
#    ./settings.h
)

# Create the executable
add_executable(abalone ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(abalone PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries (if needed)
# target_link_libraries(abalone <library_name>)