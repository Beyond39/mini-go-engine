#include <windows.h>
#include <iostream>
#include "core/Board.h"

using namespace std ;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Board game;
    Stone currentPlayer = Stone::BLACK;
    static int x, y;

    cout << "========= 围棋引擎 =========\n";
    cout << "输入格式: X Y 。 输入 -1 退出游戏。\n";
    game.printBoard();

    while (true) {
        // 提示当前该谁走
        if (currentPlayer == Stone::BLACK) cout << "\n[黑棋 Black] 请落子: ";
        else cout << "\n[白棋 White] 请落子: ";

        cin >> x;
        if (x == -1) {
            cout << "游戏结束！\n";
            break;
        }
        cin >> y;

        // 尝试落子
        if (game.playMove(x, y, currentPlayer)) {
            // 如果落子成功，打印最新棋盘，并交换对手
            cout << "\n落子成功\n";
            game.printBoard();
            
            // 交换出手权
            if (currentPlayer == Stone::BLACK) currentPlayer = Stone::WHITE;
            else currentPlayer = Stone::BLACK;
        } else {
            // 如果越界、有子、或者是自杀禁入点
            cout << "\n 违规落子， 请重新输入。\n";
        }
    }

    return 0;
}