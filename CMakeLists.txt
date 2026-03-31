cmake_minimum_required(VERSION 3.19)

project(GoEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(GoEngine
    WIN32 MACOSX_BUNDLE

    src/main.cpp

    src/ui/mainwindow/mainwindow.cpp
    src/ui/mainwindow/mainwindow.h

    src/ui/pages/HomePage.cpp
    src/ui/pages/HomePage.h
    src/ui/pages/GamePage.cpp
    src/ui/pages/GamePage.h

    src/core/Board.cpp
    src/core/Board.h
    src/core/Game.cpp
    src/core/Game.h
    src/core/sgf_parser.cpp
    src/core/sgf_parser.h
)

target_include_directories(GoEngine PRIVATE
    src/core
    src/ui/mainwindow
    src/ui/pages
    src/ui/widgets
)

target_link_libraries(GoEngine PRIVATE
    Qt6::Core
    Qt6::Widgets
)