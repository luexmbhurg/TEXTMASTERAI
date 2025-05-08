cmake_minimum_required(VERSION 3.16)
project(TextMaster VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug output
add_definitions(-DQT_DEBUG)

# Find required packages
find_package(Qt6 COMPONENTS Core Gui Widgets Concurrent REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets Concurrent REQUIRED)
endif()

# Enable automoc for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

# Add source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/llm_processor.cpp
    src/home_page.cpp
    src/flashcards_page.cpp
    src/quiz_page.cpp
    src/enumerations_page.cpp
)

# Add header files
set(HEADERS
    src/mainwindow.h
    src/llm_processor.h
    src/home_page.h
    src/flashcards_page.h
    src/quiz_page.h
    src/enumerations_page.h
)

# Add UI files
set(UI_FILES
    forms/mainwindow.ui
)

# Add resources
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Concurrent
)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
)

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Copy resources to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

# Create models directory in build folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/models
)

# Add a readme file to the models directory with download instructions
file(WRITE ${CMAKE_BINARY_DIR}/model_readme.txt 
"Place your GGUF model files in this directory.

Download models from:
- https://huggingface.co/TheBloke/Mistral-7B-v0.1-GGUF

Recommended model:
- mistral-7b-v0.1.Q4_K_M.gguf

These files are large (4GB) and not included in the repository.")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/model_readme.txt
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/models/README.txt
    )

# Copy Qt plugins
if(WIN32)
    if(Qt6_FOUND)
        set(QT_PLUGINS_DIR "${Qt6_DIR}/../../../plugins")
    else()
        set(QT_PLUGINS_DIR "${Qt5_DIR}/../../../plugins")
    endif()
    
    # Copy platform plugins
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy
        ${QT_PLUGINS_DIR}/platforms/qwindows.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/
    )
    
    # Copy style plugins
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/styles
        COMMAND ${CMAKE_COMMAND} -E copy
        ${QT_PLUGINS_DIR}/styles/qwindowsvistastyle.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/styles/
    )
    
    # Copy image format plugins
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats
        COMMAND ${CMAKE_COMMAND} -E copy
        ${QT_PLUGINS_DIR}/imageformats/qgif.dll
        ${QT_PLUGINS_DIR}/imageformats/qicns.dll
        ${QT_PLUGINS_DIR}/imageformats/qico.dll
        ${QT_PLUGINS_DIR}/imageformats/qjpeg.dll
        ${QT_PLUGINS_DIR}/imageformats/qpdf.dll
        ${QT_PLUGINS_DIR}/imageformats/qsvg.dll
        ${QT_PLUGINS_DIR}/imageformats/qtga.dll
        ${QT_PLUGINS_DIR}/imageformats/qtiff.dll
        ${QT_PLUGINS_DIR}/imageformats/qwbmp.dll
        ${QT_PLUGINS_DIR}/imageformats/qwebp.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats/
    )
endif()
