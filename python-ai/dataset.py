from pathlib import Path
import numpy as np 
import torch
from torch.utils.data import Dataset

class GoBinDataset(Dataset):

    BOARD_SIZE = 19
    CHANNELS = 3
    FEATURE_SIZE = CHANNELS * BOARD_SIZE * BOARD_SIZE  
    RECORD_SIZE = FEATURE_SIZE * 4 + 4 + 4  

    def __init__(self , bin_path: str | Path) :
        self.bin_path = Path(bin_path)

        if not self.bin_path.exists():
            raise FileNotFoundError(f"Bin file not found: {self.bin_path}")

        self.file_size = self.bin_path.stat().st_size
        if self.file_size % self.RECORD_SIZE != 0:
            raise ValueError(
                f"Invalid bin file size: {self.file_size}, "
                f"not divisible by record size {self.RECORD_SIZE}"
            )

        self.num_samples = self.file_size // self.RECORD_SIZE

    def __len__(self) -> int:
        return self.num_samples

    def __getitem__(self, idx: int):
        if idx < 0 or idx >= self.num_samples:
            raise IndexError(f"Index out of range: {idx}")

        offset = idx * self.RECORD_SIZE

        with self.bin_path.open("rb") as f:
            f.seek(offset)

            x_bytes = f.read(self.FEATURE_SIZE * 4)
            if len(x_bytes) != self.FEATURE_SIZE * 4:
                raise EOFError(f"Unexpected EOF while reading features at index {idx}")
            x_np = np.frombuffer(x_bytes, dtype=np.float32).copy()
            x = torch.from_numpy(x_np).view(self.CHANNELS, self.BOARD_SIZE, self.BOARD_SIZE)

            y_bytes = f.read(4)
            if len(y_bytes) != 4:
                raise EOFError(f"Unexpected EOF while reading policy label at index {idx}")
            y = int(np.frombuffer(y_bytes, dtype=np.int32)[0])

            z_bytes = f.read(4)
            if len(z_bytes) != 4:
                raise EOFError(f"Unexpected EOF while reading value label at index {idx}")
            z = float(np.frombuffer(z_bytes, dtype=np.float32)[0])

        return x.float(), torch.tensor(y, dtype=torch.long), torch.tensor(z, dtype=torch.float32)

