cmake_minimum_required(VERSION 3.27)
project(mpv-ytdl-gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Gui Quick DBus)
find_package(Python REQUIRED Development)
include(cmake/backtrace.cmake)

set(CMAKE_AUTORCC ON)

qt_policy(SET QTP0001 NEW)
qt_standard_project_setup()

qt_add_executable(mpv-ytdl-gui
    main.cpp mainform.cpp
    formats.cpp
    qml.qrc
)

qt_add_library(passwordhandler STATIC
    passwordhandler.cpp
)

qt_add_qml_module(mpv-ytdl-gui
    URI mpvytdlgui
    QML_FILES
        main.qml
)

include(FetchContent)
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG master
    FIND_PACKAGE_ARGS NAMES Corrosion
)
FetchContent_MakeAvailable(Corrosion)
corrosion_import_crate(MANIFEST_PATH rust/Cargo.toml)

corrosion_add_cxxbridge(mpv-ytdl-gui-rs-cxx
    CRATE mpv_ytdl_gui_rs
    FILES lib.rs
)

target_link_libraries(mpv-ytdl-gui-rs-cxx PRIVATE
    Python::Python
)

target_link_libraries(passwordhandler PUBLIC
    Qt6::DBus
)

target_link_libraries(mpv-ytdl-gui PRIVATE
    Qt6::Gui
    Qt6::Quick
    libbacktrace
    passwordhandler
    mpv-ytdl-gui-rs-cxx
)
