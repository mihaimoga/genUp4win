cmake_minimum_required(VERSION 3.15)

# DemoApp - Demo Application for genUp4win
project(DemoApp VERSION 1.0.0 LANGUAGES CXX)

# Source files
set(HEADER_FILES
    DemoApp.h
    framework.h
    pch.h
    Resource.h
    targetver.h
)

set(SOURCE_FILES
    DemoApp.cpp
    pch.cpp
)

set(RESOURCE_FILES
    DemoApp.rc
)

# Create Windows executable
add_executable(DemoApp WIN32
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# Set precompiled header
target_precompile_headers(DemoApp PRIVATE pch.h)

# Preprocessor definitions
target_compile_definitions(DemoApp PRIVATE
    _WINDOWS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Compiler options
target_compile_options(DemoApp PRIVATE
    /W3
    $<$<CONFIG:Release>:/O2>
)

# Include directories
target_include_directories(DemoApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link with genUp4win library
target_link_libraries(DemoApp PRIVATE
    genUp4win
)

# Add resource file for Windows
if(WIN32 AND RESOURCE_FILES)
    target_sources(DemoApp PRIVATE ${RESOURCE_FILES})
endif()

# Install rules
install(TARGETS DemoApp
    RUNTIME DESTINATION bin
)
