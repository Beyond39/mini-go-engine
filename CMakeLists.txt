cmake_minimum_required(VERSION 3.19)

project(GoEngine LANGUAGES CXX)

# ===== C++ 标准 =====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== Qt 路径（MinGW 版本）=====
set(CMAKE_PREFIX_PATH
    "C:/Qt/6.10.2/mingw_64"
)

# ===== 查找 Qt =====
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

qt_standard_project_setup()

# ===== 源文件 =====
qt_add_executable(GoEngine
    WIN32 MACOSX_BUNDLE

    # ===== main =====
    src/main.cpp

    # ===== UI =====
    src/ui/mainwindow/mainwindow.cpp
    src/ui/mainwindow/mainwindow.h

    src/ui/pages/HomePage.cpp
    src/ui/pages/HomePage.h
    src/ui/pages/GamePage.cpp
    src/ui/pages/GamePage.h

    src/ui/widgets/BoardWidget.cpp
    src/ui/widgets/BoardWidget.h

    # ===== core =====
    src/core/Board.cpp
    src/core/Board.h
    src/core/Game.cpp
    src/core/Game.h

    # ===== sgf =====
    src/sgf/sgf_parser.cpp
    src/sgf/sgf_parser.h
    src/sgf/sgf_writer.cpp
    src/sgf/sgf_writer.h
    src/sgf/sgf_utils.cpp
    src/sgf/sgf_utils.h

    # ===== AI =====
    src/ai/MCTS.cpp
    src/ai/MCTS.h
    src/ai/MCTSNode.cpp
    src/ai/MCTSNode.h
    src/ai/EvaluationResult.h
    src/ai/BoardSerializer.h
    src/ai/BoardSerializer.cpp
    src/ai/PythonEvaluator.h
    src/ai/PythonEvaluator.cpp
)

# ===== 头文件路径 =====
target_include_directories(GoEngine PRIVATE
    src
    src/core
    src/sgf
    src/ui/mainwindow
    src/ui/pages
    src/ui/widgets
    src/ai
)

# ===== 链接 Qt =====
target_link_libraries(GoEngine PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
)

# ===== 编译标准 =====
set_property(TARGET GoEngine PROPERTY CXX_STANDARD 17)