set(LIB_ACNH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/LibACNH")

file(GLOB SOURCES
    "${LIB_ACNH_PATH}/source/*.c*"
    )

file(GLOB HEADERS
    "${LIB_ACNH_PATH}/include/*.h*"
    )

add_library(ACNH
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(ACNH
    PUBLIC "${LIB_ACNH_PATH}/include"
    )

set(NANOVG_DEKO3D_PATH "${CMAKE_CURRENT_SOURCE_DIR}/nanovg-deko3d")

file(GLOB SOURCES
    "${NANOVG_DEKO3D_PATH}/source/*.c*"
    "${NANOVG_DEKO3D_PATH}/source/framework/*.c*"
    )

file(GLOB HEADERS
    "${NANOVG_DEKO3D_PATH}/include/*.h*"
    "${NANOVG_DEKO3D_PATH}/include/nanovg/*.h*"
    "${NANOVG_DEKO3D_PATH}/include/nanovg/framework/*.h*"
    )

find_library(NX_DEKO3D_LIBRARY
    NAMES deko3d
    HINTS "${DEVKITPRO}/portlibs/switch/lib"
    )

add_library(NanoVG_Deko3D
    ${SOURCES}
    ${HEADERS}
    )
if(NOT NX_DEKO3D_LIBRARY)
    message(FATAL_ERROR "Could not find deko3d library: try installing deko3d")
endif() 
target_link_libraries(NanoVG_Deko3D
    PRIVATE ${NX_DEKO3D_LIBRARY}
    )

target_include_directories(NanoVG_Deko3D
    PUBLIC "${NANOVG_DEKO3D_PATH}/include"
    PUBLIC "${NANOVG_DEKO3D_PATH}/include/nanovg"
    PUBLIC "${NANOVG_DEKO3D_PATH}/include/nanovg/framework"
    PRIVATE "${DEVKITPRO}/portlibs/switch/include"
    )
