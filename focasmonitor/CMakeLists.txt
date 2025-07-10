cmake_minimum_required(VERSION 3.16)

project(focasmonitor VERSION 2.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add source files
add_executable(focasmonitor 
    src/main.c
    src/connection_pool.c
    src/output.c
)

# Add build information as compile definitions
if(DEFINED ENV{GITHUB_SHA})
    set(BUILD_COMMIT $ENV{GITHUB_SHA})
else()
    execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE BUILD_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(BUILD_COMMIT STREQUAL "")
        set(BUILD_COMMIT "unknown")
    endif()
endif()

if(DEFINED ENV{BUILD_TIMESTAMP})
    set(BUILD_TIMESTAMP $ENV{BUILD_TIMESTAMP})
else()
    string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)
endif()

if(DEFINED ENV{BUILD_VERSION})
    set(BUILD_VERSION $ENV{BUILD_VERSION})
else()
    set(BUILD_VERSION "v2.0.0-dev")
endif()

target_compile_definitions(focasmonitor PRIVATE
    BUILD_COMMIT="${BUILD_COMMIT}"
    BUILD_TIMESTAMP="${BUILD_TIMESTAMP}"
    BUILD_VERSION="${BUILD_VERSION}"
)

# Dependencies
set(DEPS "fwlib32")

if (WIN32)
    # Windows build - cross-compilation setup
    add_library(fwlib32 SHARED IMPORTED)
    set_target_properties(fwlib32 PROPERTIES
        LINKER_LANGUAGE C
        PREFIX ""
        SUFFIX ""
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../Fwlib32.dll"
        IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/../Fwlib32.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/../"
    )
    
    # Copy dependency dlls to output directory
    get_filename_component(dep_dlls ${CMAKE_SOURCE_DIR} DIRECTORY)
    file(GLOB dep_dlls "${dep_dlls}/fwlib*.dll")
    list(APPEND dep_dlls "${dep_dlls}/Fwlib32.dll")
    
    foreach(dep_dll IN LISTS dep_dlls)
        if(EXISTS "${dep_dll}")
            add_custom_command(TARGET focasmonitor POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dep_dll} $<TARGET_FILE_DIR:focasmonitor>)
        endif()
    endforeach()
    
else()
    # Linux build (if needed for development/testing)
    find_library(FWLIB NAMES fwlib32 libfwlib32 HINTS "${CMAKE_SOURCE_DIR}/../" REQUIRED)
    add_library(fwlib32 SHARED IMPORTED)
    set_target_properties(fwlib32 PROPERTIES
        IMPORTED_LOCATION "${FWLIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/../")
    list(APPEND DEPS "m" "pthread")
endif()

target_link_libraries(focasmonitor ${DEPS})

# For Windows cross-compilation, explicitly link the FANUC library
if (WIN32)
    target_link_libraries(focasmonitor "${CMAKE_SOURCE_DIR}/../Fwlib32.lib")
endif()

# Include directories
target_include_directories(focasmonitor PRIVATE 
    src/
    ../
)

# Compiler warnings
if(MSVC)
    target_compile_options(focasmonitor PRIVATE /W4)
else()
    target_compile_options(focasmonitor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install rules
install(TARGETS focasmonitor
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Package DLLs for Windows
if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/../Fwlib32.dll" DESTINATION bin)
    file(GLOB FANUC_DLLS "${CMAKE_SOURCE_DIR}/../fwlib*.dll")
    install(FILES ${FANUC_DLLS} DESTINATION bin)
endif()
