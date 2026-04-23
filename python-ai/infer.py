import sys
import torch
import torch.nn.functional as F
import numpy as np

from model import SmallGoNet

BOARD_SIZE = 19

def parse_message(line: str):
    line = line.strip()
    player_str, board_str = line.split(";")

    current_player = int(player_str)
    board_values = list(map(int, board_str.split(",")))

    board = np.array(board_values, dtype=np.int32).reshape(BOARD_SIZE, BOARD_SIZE)
    return board, current_player

def board_to_tensor(board: np.ndarray, current_player: int):
    x = np.zeros((3, BOARD_SIZE, BOARD_SIZE), dtype=np.float32)

    # 通道0：黑棋
    x[0] = (board == 1).astype(np.float32)

    # 通道1：白棋
    x[1] = (board == 2).astype(np.float32)

    # 通道2：当前轮到谁
    # 这里一定要和你训练时保持一致
    # 如果你训练时是“轮到黑则全1，轮到白则全0”，那就这样写
    if current_player == 1:
        x[2][:] = 1.0
    else:
        x[2][:] = 0.0

    return torch.tensor(x, dtype=torch.float32).unsqueeze(0)


def main():
    if len(sys.argv) < 2:
        print("Need model path", file=sys.stderr)
        sys.exit(1)

    model_path = sys.argv[1]

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    model = SmallGoNet().to(device)
    state_dict = torch.load(model_path, map_location=device)
    model.load_state_dict(state_dict)
    model.eval()

    while True:
        line = sys.stdin.readline()
        if not line:
            break

        board, current_player = parse_message(line)
        x = board_to_tensor(board, current_player).to(device)

        with torch.no_grad():
            policy_logits, value_pred = model(x)
            policy_prob = torch.softmax(policy_logits[0], dim=0)
            value = value_pred[0].item()

        print(f"{value:.6f}")
        print(" ".join(f"{p:.6f}" for p in policy_prob.tolist()))
        sys.stdout.flush()



if __name__ == "__main__":
    main()