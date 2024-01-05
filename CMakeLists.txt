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

qt_add_executable(mpv-ytdl-gui main.cpp mainform.cpp formats.cpp qml.qrc ytdlp.cpp)

qt_add_qml_module(mpv-ytdl-gui
    URI mpvytdlgui
    QML_FILES
        main.qml
)

target_link_libraries(mpv-ytdl-gui PRIVATE
    Qt6::Gui
    Qt6::Quick
    Qt6::DBus
    Python::Python
    libbacktrace
)
