cmake_minimum_required(VERSION 3.16)
project(LessonDecoder VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable automoc for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add source files
set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/text_processor.cpp
    src/text_processor.h
    resources.qrc
)

# Create executable
qt_add_executable(LessonDecoder
    ${PROJECT_SOURCES}
)

# Link libraries
target_link_libraries(LessonDecoder PRIVATE
    Qt6::Core
    Qt6::Widgets
    Python3::Python
)

# Include directories
target_include_directories(LessonDecoder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Python3_INCLUDE_DIRS}
)

# Copy Python script to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/nlp_processor.py
    ${CMAKE_CURRENT_BINARY_DIR}/resources/nlp_processor.py
    COPYONLY
)

# Windows-specific settings
if(WIN32)
    # Set Windows subsystem
    set_target_properties(LessonDecoder PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Add Python DLLs to the executable
    add_custom_command(TARGET LessonDecoder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Python3::Python>
            $<TARGET_FILE_DIR:LessonDecoder>
    )
endif()
