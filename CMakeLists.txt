cmake_minimum_required(VERSION 3.14)

if(DEFINED BUILD_VERSION)
    set(PROJECT_VERSION ${BUILD_VERSION})
else()
    set(PROJECT_VERSION 0.0.0)
endif()

project(Luna
        VERSION ${PROJECT_VERSION}
        )

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_PREFIX_PATH "${DEVKITPRO};${CMAKE_PREFIX_PATH}")

set(DEBUG OFF CACHE BOOL "Enable debug mode")
set(DEBUG_OV OFF CACHE BOOL "Enable debug overlay mode")
set(DEBUG_PRINTF OFF CACHE BOOL "Enable debug printing to NXLink console")
set(DEBUG_UI OFF CACHE BOOL "Enable debug mode of UI (disable game and template checks)")

if(DEBUG)
 add_definitions(-DDEBUG)
endif(DEBUG)
if(DEBUG_OV)
 add_definitions(-DDEBUG_OV)
endif(DEBUG_OV)
if(DEBUG_PRINTF)
 add_definitions(-DDEBUG_PRINTF)
endif(DEBUG_PRINTF)
if(DEBUG_UI)
 add_definitions(-DDEBUG_UI)
endif(DEBUG_UI)

# include subdirectory
add_subdirectory(libs)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/source/version.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/version.h"
    @ONLY
)

file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/helpers/**/*.c*"
    )

file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/helpers/**/*.h*"
)
    
add_executable(Luna
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(Luna
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/source"
    )
    
target_link_libraries(Luna
    PRIVATE ACNH
    PRIVATE NanoVG_Deko3D
    )

nx_generate_nacp(Luna
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/luna.nacp"
    NAME "Luna"
    AUTHOR "Ixaruz"
    VERSION ${PROJECT_VERSION}
    )
    
nx_create_nro(Luna
    VERSION ${PROJECT_VERSION}
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/luna.nro"
    NACP "${CMAKE_CURRENT_BINARY_DIR}/luna.nacp"
    ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg"
    ROMFS "${CMAKE_CURRENT_SOURCE_DIR}/romfs"
    )
