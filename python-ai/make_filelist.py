from pathlib import Path
import random

TOTAL_TRAIN = 10000
TOTAL_VAL = 600
TOTAL_TEST = 600

RANDOM_SEED = 42   


def collect_sgf_files(root: Path) -> list[Path]:
    print(f"扫描目录: {root}")
    files = list(root.rglob("*.sgf"))
    print(f"找到 SGF 文件数量: {len(files)}")
    return files


def print_subdir_stats(root: Path) -> None:
    print("\n子目录统计：")
    for subdir in sorted([p for p in root.iterdir() if p.is_dir()]):
        count = len(list(subdir.rglob("*.sgf")))
        print(f"{subdir.name}: {count}")


def save_file_list(paths: list[Path], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)

    with out_path.open("w", encoding="utf-8") as f:
        for p in paths:
            f.write(str(p) + "\n")

    print(f"已写入: {out_path} (共 {len(paths)} 条)")


def main():
    project_root = Path(__file__).resolve().parent.parent
    sgf_root = project_root / "data" / "train_sgf"
    filelist_dir = project_root / "data" / "filelists"

    if not sgf_root.exists():
        raise FileNotFoundError(f"找不到目录: {sgf_root}")

    print_subdir_stats(sgf_root)

    all_files = collect_sgf_files(sgf_root)

    required = TOTAL_TRAIN + TOTAL_VAL + TOTAL_TEST
    if len(all_files) < required:
        raise ValueError(
            f"SGF 数量不足，需要 {required}，实际只有 {len(all_files)}"
        )

    random.seed(RANDOM_SEED)
    random.shuffle(all_files)

    train_files = all_files[:TOTAL_TRAIN]
    val_files = all_files[TOTAL_TRAIN:TOTAL_TRAIN + TOTAL_VAL]
    test_files = all_files[TOTAL_TRAIN + TOTAL_VAL:TOTAL_TRAIN + TOTAL_VAL + TOTAL_TEST]

    print("\n划分结果：")
    print(f"train: {len(train_files)}")
    print(f"val  : {len(val_files)}")
    print(f"test : {len(test_files)}")
    print(f"seed : {RANDOM_SEED}")

    save_file_list(train_files, filelist_dir / "train_10k.txt")
    save_file_list(val_files, filelist_dir / "val_600.txt")
    save_file_list(test_files, filelist_dir / "test_600.txt")

    print("\n文件列表生成完成。")


if __name__ == "__main__":
    main()