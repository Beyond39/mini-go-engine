from pathlib import Path
from dataset import GoBinDataset


def main():
    project_root = Path(__file__).resolve().parent.parent
    bin_path = project_root / "data" / "bin" / "train_10k.bin"

    dataset = GoBinDataset(bin_path)

    print(f"Bin file: {bin_path}")
    print(f"Number of samples: {len(dataset)}")

    x, y, z = dataset[0]

    print("First sample:")
    print("x.shape =", x.shape)
    print("y =", y.item())
    print("z =", z.item())


if __name__ == "__main__":
    main()