
cmake_minimum_required(VERSION 3.10)
project(TextMaster VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required Qt packages
find_package(Qt5 COMPONENTS Core Widgets Multimedia MultimediaWidgets REQUIRED)

# Configure resources
configure_file(resources.qrc ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc COPYONLY)

# Add resource files
set(RESOURCE_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc
)

# Add source files
set(SOURCE_FILES
    src/main.cpp
    src/mainwindow.cpp
)

# Add header files
set(HEADER_FILES
    src/mainwindow.h
)

# Add Python scripts to resources
set(PYTHON_SCRIPTS
    src/resources/nlp_processor.py
)

# Generate Python module directory
add_custom_target(PyModules ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_SCRIPTS} ${CMAKE_BINARY_DIR}/resources/
)

# Add embedded Python support
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})

# Add executable
add_executable(TextMaster ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# Link libraries
target_link_libraries(TextMaster PRIVATE 
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Multimedia 
    Qt5::MultimediaWidgets
    ${Python3_LIBRARIES}
)

# Set include directories
target_include_directories(TextMaster PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Python3_INCLUDE_DIRS}
)

# Install target
install(TARGETS TextMaster DESTINATION bin)
install(FILES ${PYTHON_SCRIPTS} DESTINATION resources)

# Set Windows-specific properties
if(WIN32)
    # Set application icon
    set_target_properties(TextMaster PROPERTIES WIN32_EXECUTABLE TRUE)
    
    # PyInstaller command for creating Windows executable
    add_custom_command(TARGET TextMaster POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Creating Windows executable package"
        COMMAND ${Python3_EXECUTABLE} -m pip install pyinstaller
        COMMAND ${Python3_EXECUTABLE} -m PyInstaller --onefile --windowed --name TextMaster
                --add-data "${CMAKE_BINARY_DIR}/resources/nlp_processor.py;resources/"
                --add-binary "${CMAKE_BINARY_DIR}/TextMaster.exe;."
                "${CMAKE_SOURCE_DIR}/windows_package.py"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
