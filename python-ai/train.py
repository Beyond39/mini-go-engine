from pathlib import Path

import torch
import torch.nn.functional as F
from torch.utils.data import DataLoader

from dataset import GoBinDataset
from model import SmallGoNet


def evaluate(model, dataloader, device):
    model.eval()

    total_loss = 0.0
    total_policy_loss = 0.0
    total_value_loss = 0.0
    correct = 0
    total = 0

    with torch.no_grad():
        for x, y, z in dataloader:
            x = x.to(device, non_blocking=True)
            y = y.to(device, non_blocking=True)
            z = z.to(device, non_blocking=True)

            policy_logits, value_pred = model(x)

            policy_loss = F.cross_entropy(policy_logits, y)
            value_loss = F.mse_loss(value_pred, z)
            loss = policy_loss + value_loss

            total_loss += loss.item()
            total_policy_loss += policy_loss.item()
            total_value_loss += value_loss.item()

            pred = torch.argmax(policy_logits, dim=1)
            correct += (pred == y).sum().item()
            total += y.size(0)

    avg_loss = total_loss / len(dataloader)
    avg_policy_loss = total_policy_loss / len(dataloader)
    avg_value_loss = total_value_loss / len(dataloader)
    acc = correct / total if total > 0 else 0.0

    return avg_loss, avg_policy_loss, avg_value_loss, acc


def train():
    project_root = Path(__file__).resolve().parent.parent
    data_dir = project_root / "data" / "bin"
    ckpt_dir = project_root / "python-ai" / "checkpoints"
    ckpt_dir.mkdir(parents=True, exist_ok=True)

    train_bin = data_dir / "train_10k.bin"
    val_bin = data_dir / "val_600.bin"

    if not train_bin.exists():
        raise FileNotFoundError(f"Train bin not found: {train_bin}")
    if not val_bin.exists():
        raise FileNotFoundError(f"Val bin not found: {val_bin}")

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")

    train_dataset = GoBinDataset(train_bin)
    val_dataset = GoBinDataset(val_bin)

    print(f"Train samples: {len(train_dataset)}")
    print(f"Val samples: {len(val_dataset)}")

    batch_size = 64
    epochs = 5
    log_interval = 500

    train_loader = DataLoader(
        train_dataset,
        batch_size=batch_size,
        shuffle=True,
        num_workers=0,
        pin_memory=(device.type == "cuda"),
    )

    val_loader = DataLoader(
        val_dataset,
        batch_size=batch_size,
        shuffle=False,
        num_workers=0,
        pin_memory=(device.type == "cuda"),
    )

    model = SmallGoNet().to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-3)

    best_val_loss = float("inf")

    print("Start training...")
    print(f"Total train batches per epoch: {len(train_loader)}")
    print(f"Total val batches per epoch: {len(val_loader)}")
    print("-" * 80)

    for epoch in range(1, epochs + 1):
        model.train()

        running_loss = 0.0
        running_policy_loss = 0.0
        running_value_loss = 0.0

        for batch_idx, (x, y, z) in enumerate(train_loader, start=1):
            x = x.to(device, non_blocking=True)
            y = y.to(device, non_blocking=True)
            z = z.to(device, non_blocking=True)

            policy_logits, value_pred = model(x)

            policy_loss = F.cross_entropy(policy_logits, y)
            value_loss = F.mse_loss(value_pred, z)
            loss = policy_loss + value_loss

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

            running_loss += loss.item()
            running_policy_loss += policy_loss.item()
            running_value_loss += value_loss.item()

            if batch_idx % log_interval == 0 or batch_idx == len(train_loader):
                avg_so_far = running_loss / batch_idx
                avg_policy_so_far = running_policy_loss / batch_idx
                avg_value_so_far = running_value_loss / batch_idx

                print(
                    f"Epoch {epoch:02d} | "
                    f"batch {batch_idx:05d}/{len(train_loader):05d} | "
                    f"avg_loss={avg_so_far:.4f} | "
                    f"avg_policy={avg_policy_so_far:.4f} | "
                    f"avg_value={avg_value_so_far:.4f}"
                )

        train_loss = running_loss / len(train_loader)
        train_policy_loss = running_policy_loss / len(train_loader)
        train_value_loss = running_value_loss / len(train_loader)

        val_loss, val_policy_loss, val_value_loss, val_acc = evaluate(
            model, val_loader, device
        )

        print("-" * 80)
        print(
            f"Epoch {epoch:02d} finished | "
            f"train_loss={train_loss:.4f} "
            f"(policy={train_policy_loss:.4f}, value={train_value_loss:.4f}) | "
            f"val_loss={val_loss:.4f} "
            f"(policy={val_policy_loss:.4f}, value={val_value_loss:.4f}) | "
            f"val_policy_acc={val_acc:.4f}"
        )

        latest_path = ckpt_dir / "go_model_latest.pth"
        torch.save(model.state_dict(), latest_path)
        print(f"Saved latest model to: {latest_path}")

        if val_loss < best_val_loss:
            best_val_loss = val_loss
            best_path = ckpt_dir / "go_model_best.pth"
            torch.save(model.state_dict(), best_path)
            print(f"Saved best model to: {best_path}")

        print("=" * 80)

    print("Training finished.")


if __name__ == "__main__":
    train()