cmake_minimum_required(VERSION 3.10)
project(TextMaster)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(OpenGL REQUIRED)

# Add GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# Add Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

# Add source files
set(SOURCES
    src/main.cpp
)

# Add header files
set(HEADERS
    src/imgui.h
    src/imgui_impl_glfw.h
    src/imgui_impl_opengl3.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    imgui
    OpenGL::GL
    Python3::Python
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Python3_INCLUDE_DIRS}
)

# Copy Python script to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/nlp_processor.py
    ${CMAKE_CURRENT_BINARY_DIR}/nlp_processor.py
    COPYONLY
)

# Windows-specific settings
if(WIN32)
    # Set Windows subsystem
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Add Windows-specific libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        gdi32
        shell32
    )
endif()
