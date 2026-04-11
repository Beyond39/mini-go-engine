cmake_minimum_required(VERSION 3.19)

project(GoEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

    src/ui/widgets/BoardWidget.cpp
    src/ui/widgets/BoardWidget.h

    src/core/Board.cpp
    src/core/Board.h
    src/core/Game.cpp
    src/core/Game.h

    src/sgf/sgf_parser.cpp
    src/sgf/sgf_parser.h
    src/sgf/sgf_writer.cpp
    src/sgf/sgf_writer.h
    src/sgf/sgf_utils.cpp
    src/sgf/sgf_utils.h

    src/ai/MCTS.cpp
    src/ai/MCTS.h
    src/ai/MCTSNode.cpp
    src/ai/MCTSNode.h
)

target_include_directories(GoEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sgf
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/mainwindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
)

target_link_libraries(GoEngine PRIVATE
    Qt6::Core
    Qt6::Widgets
)