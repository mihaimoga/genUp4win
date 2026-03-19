cmake_minimum_required(VERSION 3.15)

project(GenericUpdater VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows-specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Add subdirectories
add_subdirectory(genUp4win)
add_subdirectory(DemoApp)

# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DemoApp)

# Instructions for building outside of the IDE
if(MSVC)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "@echo off\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "mkdir build\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "cd build\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "cmake -G \"Visual Studio 17 2022\" ..\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "cmake --build . --config Release\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "cd ..\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/build.bat" "pause\n")
    file(COPY "${CMAKE_CURRENT_BINARY_DIR}/build.bat" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
