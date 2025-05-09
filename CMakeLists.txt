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

# llama.cpp configuration
set(LLAMA_STATIC ON CACHE BOOL "Build llama.cpp as static library" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(LLAMA_NATIVE OFF CACHE BOOL "Build with native CPU optimizations" FORCE)
set(LLAMA_AVX OFF CACHE BOOL "Enable AVX" FORCE)
set(LLAMA_AVX2 ON CACHE BOOL "Enable AVX2" FORCE)
set(LLAMA_FMA ON CACHE BOOL "Enable FMA" FORCE)
set(LLAMA_F16C ON CACHE BOOL "Enable F16C" FORCE)

# Add llama.cpp subdirectory
add_subdirectory(llama.cpp/llama.cpp-master)

# Add source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/llm_processor.cpp
    src/home_page.cpp
    src/flashcards_page.cpp
    src/quiz_page.cpp
    src/enumerations_page.cpp
    src/pages/results_page.cpp
)

# Add header files
set(HEADERS
    src/mainwindow.h
    src/llm_processor.h
    src/home_page.h
    src/flashcards_page.h
    src/quiz_page.h
    src/enumerations_page.h
    src/pages/results_page.h
)

# Add UI files
set(UI_FILES
    forms/mainwindow.ui
    forms/homepage.ui
    forms/quizpage.ui
    forms/flashcardspage.ui
    forms/enumerationspage.ui
    forms/historypage.ui
    forms/resultspage.ui
)

# Add resources
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

# Link against static libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    llama
    ggml
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp/llama.cpp-master/src
)

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Create model README file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/model_readme.txt 
"Place your GGUF model files in this directory.

Download models from:
- https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF

Recommended model:
- tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf

This file is ~638MB and not included in the repository.")

# Add post-build commands to copy resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Copying resources"
)

# Add post-build commands to copy model files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/models
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/model_readme.txt
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/models/README.txt
    COMMENT "Copying model files"
)

# Add post-build commands to copy Qt plugins
if(WIN32)
    set(QT_PLUGIN_PATH "${Qt6_DIR}/../../../plugins")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_PLUGIN_PATH}/platforms/qwindows.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/
        COMMENT "Copying Qt platform plugins"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/styles
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_PLUGIN_PATH}/styles/qwindowsvistastyle.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/styles/
        COMMENT "Copying Qt style plugins"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_PLUGIN_PATH}/imageformats/qgif.dll"
            "${QT_PLUGIN_PATH}/imageformats/qicns.dll"
            "${QT_PLUGIN_PATH}/imageformats/qico.dll"
            "${QT_PLUGIN_PATH}/imageformats/qjpeg.dll"
            "${QT_PLUGIN_PATH}/imageformats/qpdf.dll"
            "${QT_PLUGIN_PATH}/imageformats/qsvg.dll"
            "${QT_PLUGIN_PATH}/imageformats/qtga.dll"
            "${QT_PLUGIN_PATH}/imageformats/qtiff.dll"
            "${QT_PLUGIN_PATH}/imageformats/qwbmp.dll"
            "${QT_PLUGIN_PATH}/imageformats/qwebp.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats/
        COMMENT "Copying Qt image format plugins"
    )
endif()
