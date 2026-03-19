cmake_minimum_required(VERSION 3.15)

# genUp4win - Generic Updater Library
project(genUp4win VERSION 1.0.0 LANGUAGES CXX)

# Source files
set(HEADER_FILES
    ../AppSettings.h
    ../framework.h
    ../genUp4win.h
    ../pch.h
    ../resource.h
    ../SHA256.h
    ../VersionInfo.h
)

set(SOURCE_FILES
    ../dllmain.cpp
    ../genUp4win.cpp
    ../pch.cpp
    ../SHA256.cpp
    ../VersionInfo.cpp
)

set(RESOURCE_FILES
    ../genUp4win.rc
)

# Create shared library (DLL)
add_library(genUp4win SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# Set precompiled header
target_precompile_headers(genUp4win PRIVATE ../pch.h)

# Preprocessor definitions
target_compile_definitions(genUp4win PRIVATE
    GENUP4WIN_EXPORTS
    _WINDOWS
    _USRDLL
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Compiler options
target_compile_options(genUp4win PRIVATE
    /W3
    $<$<CONFIG:Release>:/O2>
)

# Include directories
target_include_directories(genUp4win PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Linker options for Windows
if(WIN32)
    set_target_properties(genUp4win PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
    target_link_libraries(genUp4win PRIVATE
        urlmon.lib
    )
endif()

# Add resource file for Windows
if(WIN32 AND RESOURCE_FILES)
    target_sources(genUp4win PRIVATE ${RESOURCE_FILES})
endif()

# Install rules
install(TARGETS genUp4win
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ../genUp4win.h ../AppSettings.h ../VersionInfo.h
    DESTINATION include
)
