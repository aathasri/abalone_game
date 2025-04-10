cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Abalone VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

## Enable AddressSanitizer
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Add static linking flags
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
#  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

# Add pybind11
#  add_subdirectory(pybind11)

# Add the source files
set(SOURCES
        src/main.cpp
        src/board.cpp
        src/move.cpp
        src/move_generator.cpp
        src/board_generator.cpp
        src/heuristic_calculator.cpp
        src/minimax.cpp
        src/settings.cpp
        src/game.cpp
        # src/abalone_wrapper.cpp
        src/threadpool.cpp
)

# Add the header files (optional, for IDEs)
set(HEADERS
        include/board.h
        include/move.h
        include/move_generator.h
        include/board_generator.h
        include/heuristic_calculator.h
        include/minimax.h
        include/settings.h
        include/game.h
        include/threadpool.h
#        include/abalone_wrapper.h
)
#
## Create the executable
add_executable(abalone ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(abalone PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Create the Python module
#  pybind11_add_module(abalone_cpp ${SOURCES} ${HEADERS})

# Include directories - pybind11
#  target_include_directories(abalone_cpp PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries (if needed)
# target_link_libraries(abalone <library_name>)