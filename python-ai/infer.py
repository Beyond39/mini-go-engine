import sys
import torch
import torch.nn.functional as F

from model import SmallGoNet


BOARD_SIZE = 19


def load_board(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]

    current_player = int(lines[0])
    board = []

    for i in range(1, BOARD_SIZE + 1):
        row = list(map(int, lines[i].split()))
        board.append(row)

    return current_player, board


def encode_board(current_player, board):
    opponent = 2 if current_player == 1 else 1

    tensor = torch.zeros((1, 3, BOARD_SIZE, BOARD_SIZE), dtype=torch.float32)

    for y in range(BOARD_SIZE):
        for x in range(BOARD_SIZE):
            stone = board[y][x]
            if stone == current_player:
                tensor[0, 0, y, x] = 1.0
            elif stone == opponent:
                tensor[0, 1, y, x] = 1.0

    if current_player == 1:
        x[0, 2, :, :] = 1.0
    else:
        x[0, 2, :, :] = 0.0

    return x


def main():
    if len(sys.argv) < 3:
        print("0.0")
        sys.exit(1)

    board_file = sys.argv[1]
    model_path = sys.argv[2]

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    model = SmallGoNet().to(device)   # 改成你的模型构造
    state_dict = torch.load(model_path, map_location=device)
    model.load_state_dict(state_dict)
    model.eval()

    current_player, board = load_board(board_file)
    x = encode_board(current_player, board).to(device)

    with torch.no_grad():
        policy_logits, value = model(x)

        policy_prob = F.softmax(policy_logits, dim=1)[0]   # [362]
        value = float(value[0].item())

    print(f"{value:.6f}")
    print(" ".join(f"{p:.8f}" for p in policy_prob.tolist()))


if __name__ == "__main__":
    main()