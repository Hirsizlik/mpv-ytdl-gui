include(ExternalProject)

ExternalProject_Add(project_libbacktrace
    PREFIX libbacktrace
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/libbacktrace
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libbacktrace
    CONFIGURE_COMMAND "${CMAKE_CURRENT_LIST_DIR}/../external/libbacktrace/configure"
                      "--prefix=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace"
                      --with-pic
                      "CC=${c_compiler}"
                      "CFLAGS=${CMAKE_C_FLAGS}"
                      "LDFLAGS=${CMAKE_EXE_LINKER_FLAGS}"
                      "CPP=${c_compiler}"
                      "NM=${CMAKE_NM}"
                      "STRIP=${CMAKE_STRIP}"
                      "--host=${MACHINE_NAME}"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace"
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/lib/libbacktrace.a"
                     "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/include/backtrace.h"
)

add_library(libbacktrace INTERFACE)
add_dependencies(libbacktrace project_libbacktrace)
target_include_directories(libbacktrace INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/include)
target_link_libraries(libbacktrace INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/lib/libbacktrace.a")
