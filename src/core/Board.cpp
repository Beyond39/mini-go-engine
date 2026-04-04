#include "Board.h"

bool Board::operator==(const Board& other) const{
    for (int i = 0; i < Board::SIZE; ++i) {
        for (int j = 0; j < Board::SIZE; ++j) {
            if (board[i][j] != other.board[i][j]) {
                return false;
            }
        }
    }
    return true; 
}

bool Board::operator!=(const Board& other) const {
    return !(*this == other);
}

// 构造函数：初始化空棋盘
Board::Board() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            board[i][j] = Stone::EMPTY;
        }
    }
}

// 边界检查
bool Board::inBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < SIZE && y < SIZE;
}

// 获取棋子颜色
Stone Board::get(int x, int y) const {
    if (!inBounds(x, y)) {
        return Stone::EMPTY; 
    }
    return board[x][y];
}

// 获取对手颜色
Stone Board::opponent(Stone color) const {
    if (color == Stone::BLACK){
        return Stone::WHITE;
    }
    if (color == Stone::WHITE){
        return Stone::BLACK;
    }
    return Stone::EMPTY;
}

// 算气函数，计算某一个或者某一块棋的气，这是核心的函数 
bool Board::hasLiberty(int x, int y, bool visited[SIZE][SIZE]) const {
    visited[x][y] = true;         // 标记当前棋子已查过
    Stone color = board[x][y];

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (!inBounds(nx, ny)) {
            continue;
        }

        if (board[nx][ny] == Stone::EMPTY) {
            return true;
        }

        if (board[nx][ny] == color && !visited[nx][ny]) {
            if (hasLiberty(nx, ny, visited)) {
                return true; 
            }
        }
    }
    return false;
}

// 提子函数
int Board::removeGroup(int x, int y, Stone color) {
    board[x][y] = Stone::EMPTY; 
    int removedCount = 1;      

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        if (inBounds(nx, ny) && board[nx][ny] == color) {
            removedCount += removeGroup(nx, ny, color);
        }
    }
    return removedCount;
}

void Board::collectGroup(int x, int y, Stone color,bool visited[SIZE][SIZE], std::vector<std::pair<int, int>>& group) const {
    visited[x][y] = true;
    group.push_back({x, y});

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (inBounds(nx, ny) && !visited[nx][ny] && board[nx][ny] == color) {
            collectGroup(nx, ny, color, visited, group);
        }
    }
}

std::vector<std::pair<int, int>> Board::getLegalMoves(Stone color) const{
    std::vector<std::pair<int,int>> legalMove ;

    for (int x = 0 ; x < Board::SIZE ; ++x){
        for (int y = 0 ; y < Board::SIZE ; ++y){
            if(isLegalMove(x,y,color)){
                legalMove.push_back({x,y}) ;
            }
        }
    }

    return legalMove ;
}

// 落子的逻辑
bool Board::playMove(int x, int y, Stone color) {
    if (!inBounds(x, y) || board[x][y] != Stone::EMPTY) {
        return false;
    }

    board[x][y] = color;
    Stone opp = opponent(color);
    bool captured = false ;

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (inBounds(nx, ny) && board[nx][ny] == opp) {
            bool visited[SIZE][SIZE] = {false}; 
            if (!hasLiberty(nx, ny, visited)) {
                removeGroup(nx, ny, opp);
                captured = true ;
            }
        }
    }

    bool visitedSelf[SIZE][SIZE] = {false};
    if (!captured && !hasLiberty(x, y, visitedSelf)) {
        board[x][y] = Stone::EMPTY;
        return false;
    }

    return true; 
}

bool Board::isLegalMove(int x, int y, Stone color) const{
    Board tmp = *this ;
    return tmp.playMove(x,y,color) ;
}

// 调试打印 
void Board::printBoard() const {
    std::cout << "  ";
    for (int i = 0; i < SIZE; i++) std::cout << i % 10 << " ";
    std::cout << "\n";
    for (int y = 0; y < SIZE; ++y) {
        std::cout << y % 10 << " ";
        for (int x = 0; x < SIZE; ++x) {
            if (board[x][y] == Stone::EMPTY) std::cout << ". ";
            else if (board[x][y] == Stone::BLACK) std::cout << "X ";
            else std::cout << "O ";
        }
        std::cout << "\n";
    }
}