cmake_minimum_required (VERSION 2.6)
project (SerialTool)

set(SerialTool_VERSION_MAJOR 0)
set(SerialTool_VERSION_MINOR 1)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/SerialToolConfig.h.in
    ${PROJECT_BINARY_DIR}/SerialToolConfig.h
)

set(
    HEADER_FILES
    ${PROJECT_SOURCE_DIR}/include/serial_utils.h
    ${PROJECT_SOURCE_DIR}/include/string_utils.h
)

set(
    SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/serial_utils.c
    ${PROJECT_SOURCE_DIR}/src/string_utils.c
    ${PROJECT_SOURCE_DIR}/src/main.c
)

find_path(SERIAL_PORT_INCLUDE "serialport")
find_library(SERIAL_PORT_LIB serialport REQUIRED)

find_path(IUP_INCLUDE "iup")
find_library(IUP_LIB iup REQUIRED)
find_library(IUP_CONTROLS_LIB iupcontrols REQUIRED)
find_library(IUP_CD_LIB iupcd REQUIRED)

if (UNIX)
    set(
        EXTRA_LIBS ${EXTRA_LIBS} ${SERIAL_PORT_LIB} ${IUP_LIB}
        ${IUP_CONTROLS_LIB} ${IUP_CD_LIB} 
        cd im gtk-3 gdk-3 gdk_pixbuf-2.0 pangocairo-1.0 
        pango-1.0 cairo gobject-2.0 gmodule-2.0 glib-2.0
        Xext X11 m fontconfig freetype
    )
else()
    set(
        EXTRA_LIBS ${EXTRA_LIBS} ${SERIAL_PORT_LIB} ${IUP_LIB}
        ${IUP_CONTROLS_LIB} ${IUP_CD_LIB})
endif()

if(IUP_INCLUDE AND SERIAL_PORT_INCLUDE)
    add_executable(SerialTool ${SOURCE_FILES} ${HEADER_FILES})
    target_link_libraries(SerialTool ${EXTRA_LIBS})
    target_include_directories(
        SerialTool PUBLIC
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${IUP_INCLUDE}
        ${SERIAL_PORT_INCLUDE}
    )
endif()