cmake_minimum_required(VERSION 3.10)
project(AbaloneGame)

set(CMAKE_CXX_STANDARD 17)

# Set the path to Qt (update if needed)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt/lib/cmake")

# Find Qt components (Widgets for GUI applications)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Change the output file name to "game"
add_executable(game src/main.cpp)

# Link the Qt libraries
target_link_libraries(game PRIVATE Qt6::Widgets)