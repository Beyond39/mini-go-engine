cmake_minimum_required(VERSION 3.19)

project(GoEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_PREFIX_PATH
    "C:/Qt/6.10.2/mingw_64"
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent Multimedia)

qt_standard_project_setup()

set(CORE_SOURCES
    src/core/Board.cpp
    src/core/Board.h
    src/core/Game.cpp
    src/core/Game.h
)

set(SGF_SOURCES
    src/sgf/sgf_parser.cpp
    src/sgf/sgf_parser.h
    src/sgf/sgf_writer.cpp
    src/sgf/sgf_writer.h
    src/sgf/sgf_utils.cpp
    src/sgf/sgf_utils.h
)

set(AI_SOURCES
    src/ai/MCTS.cpp
    src/ai/MCTS.h
    src/ai/MCTSNode.cpp
    src/ai/MCTSNode.h
    src/ai/EvaluationResult.h
    src/ai/BoardSerializer.cpp
    src/ai/BoardSerializer.h
    src/ai/PythonEvaluator.cpp
    src/ai/PythonEvaluator.h
)

set(UI_SOURCES
    src/ui/mainwindow/mainwindow.cpp
    src/ui/mainwindow/mainwindow.h

    src/ui/pages/HomePage.cpp
    src/ui/pages/HomePage.h
    src/ui/pages/GamePage.cpp
    src/ui/pages/GamePage.h
    src/ui/pages/WinRateChartWidget.cpp
    src/ui/pages/WinRateChartWidget.h
    src/ui/pages/ReviewDialog.cpp
    src/ui/pages/ReviewDialog.h

    src/ui/widgets/BoardWidget.cpp
    src/ui/widgets/BoardWidget.h
)

qt_add_executable(GoEngine
    WIN32
    src/main.cpp
    ${CORE_SOURCES}
    ${SGF_SOURCES}
    ${AI_SOURCES}
    ${UI_SOURCES}
)

target_include_directories(GoEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sgf
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/mainwindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
)

target_link_libraries(GoEngine PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Multimedia
)

target_compile_definitions(GoEngine PRIVATE Q_CC_GHS)

set_property(TARGET GoEngine PROPERTY CXX_STANDARD 17)

add_executable(extract_main
    tools/extract_main.cpp
    tools/DataExtractor.cpp
    tools/DataExtractor.h

    ${CORE_SOURCES}

    src/sgf/sgf_parser.cpp
    src/sgf/sgf_parser.h
    src/sgf/sgf_utils.cpp
    src/sgf/sgf_utils.h
)

target_include_directories(extract_main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sgf
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
)

target_link_libraries(extract_main PRIVATE
    Qt6::Core
)

target_compile_definitions(extract_main PRIVATE Q_CC_GHS)

set_property(TARGET extract_main PROPERTY CXX_STANDARD 17)