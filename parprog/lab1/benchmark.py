#!/usr/bin/env python3
import argparse
import csv
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Tuple

import numpy as np


METHODS = [
    {"name": "slow", "label": "SLOW", "binary": "slow"},
    {"name": "trancepose", "label": "TRANSPOSE", "binary": "trancepose"},
    {"name": "block", "label": "BLOCK_TRANSPOSE", "binary": "block"},
    {"name": "strass", "label": "STRASSEN_TRANSPOSE", "binary": "strass"},
    {"name": "parallel", "label": "PARALLEL_STRASSEN_TRANSPOSE", "binary": "parallel"},
    {"name": "simd", "label": "SIMD_PARALLEL_STRASSEN_TRANSPOSE", "binary": "simd"},
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Benchmark all matrix multiplication implementations and plot time vs size.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--sizes",
        nargs="+",
        type=int,
        help="Explicit list of matrix sizes to test (overrides min/max).",
    )
    parser.add_argument("--min-size", type=int, default=32, help="Minimum matrix dimension.")
    parser.add_argument("--max-size", type=int, default=2048, help="Maximum matrix dimension.")
    parser.add_argument(
        "--methods",
        nargs="+",
        choices=[m["name"] for m in METHODS],
        help="Subset of methods to run (default: run all).",
    )
    parser.add_argument(
        "--regen-inputs",
        action="store_true",
        help="Regenerate input matrices even if cached files already exist.",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip rebuilding binaries with make.",
    )
    parser.add_argument("--seed", type=int, default=0, help="Seed for random matrix generation.")
    parser.add_argument(
        "--log-file",
        type=Path,
        default=Path("benchmark_timings.log"),
        help="Where to append raw timing records.",
    )
    parser.add_argument(
        "--csv-file",
        type=Path,
        default=Path("benchmark_timings.csv"),
        help="Where to write parsed timing CSV.",
    )
    parser.add_argument(
        "--plot-file",
        type=Path,
        default=Path("benchmark_timings.png"),
        help="Where to save the timing plot.",
    )
    return parser.parse_args()


def size_list(args: argparse.Namespace) -> List[int]:
    if args.sizes:
        sizes = [int(s) for s in args.sizes]
    else:
        sizes = [2 ** p for p in range(5, 12) if args.min_size <= 2 ** p <= args.max_size]
    if not sizes:
        raise ValueError("No matrix sizes selected. Check --min-size/--max-size/--sizes.")
    return sizes


def write_matrix(path: Path, matrix: np.ndarray) -> None:
    flat = matrix.ravel()
    with path.open("w") as f:
        f.write(f"{matrix.shape[0]}\n")
        # Write in chunks to avoid holding a giant string in memory for large matrices.
        for start in range(0, flat.size, 1024):
            chunk = flat[start : start + 1024]
            f.write(" ".join(str(int(val)) for val in chunk))
            f.write(" ")


def generate_inputs(size: int, data_dir: Path, rng: np.random.Generator, regen: bool) -> Tuple[Path, Path]:
    data_dir.mkdir(parents=True, exist_ok=True)
    a_path = data_dir / f"A_{size}.dat"
    b_path = data_dir / f"B_{size}.dat"

    if regen or not (a_path.exists() and b_path.exists()):
        print(f"[data] Generating inputs for size {size}")
        A = rng.integers(0, 256, size=(size, size), dtype=np.uint32)
        B = rng.integers(0, 256, size=(size, size), dtype=np.uint32)
        write_matrix(a_path, A)
        write_matrix(b_path, B)
    else:
        print(f"[data] Reusing cached inputs for size {size}")

    return a_path, b_path


def build_binaries(base_dir: Path, skip_build: bool) -> None:
    if skip_build:
        return
    print("[build] Running make")
    subprocess.run(["make"], cwd=base_dir, check=True)


def run_method(
    method: Dict,
    size: int,
    a_path: Path,
    b_path: Path,
    output_dir: Path,
    build_dir: Path,
    log_file: Path,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    binary = build_dir / method["binary"]
    if not binary.exists():
        raise FileNotFoundError(f"Binary not found: {binary}. Did you run make?")

    out_path = output_dir / f"{method['name']}_{size}.dat"
    cmd = [str(binary), str(a_path), str(b_path), str(out_path), str(log_file)]
    print(f"[run] {method['label']:28s} size={size}")
    subprocess.run(cmd, check=True)


def parse_log(log_file: Path) -> List[dict]:
    records: List[dict] = []
    if not log_file.exists():
        return records

    with log_file.open() as f:
        for line in f:
            parts = line.strip().split(",")
            if len(parts) != 3:
                continue
            method, size_str, seconds_str = parts
            try:
                records.append({"method": method, "size": int(size_str), "seconds": float(seconds_str)})
            except ValueError:
                continue
    return records


def write_csv(records: List[dict], csv_file: Path) -> None:
    csv_file.parent.mkdir(parents=True, exist_ok=True)
    with csv_file.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["method", "size", "seconds"])
        for row in sorted(records, key=lambda r: (r["method"], r["size"])):
            writer.writerow([row["method"], row["size"], f"{row['seconds']:.9f}"])


def plot(records: List[dict], plot_file: Path) -> None:
    try:
        import matplotlib.pyplot as plt
    except ImportError:  # pragma: no cover - only triggered when matplotlib missing
        print("matplotlib is not installed; skipping plot. Install it with `pip install matplotlib`.", file=sys.stderr)
        return

    plot_file.parent.mkdir(parents=True, exist_ok=True)
    plt.figure(figsize=(10, 6))
    methods = sorted({r["method"] for r in records})
    if not methods:
        print("No timing data found; skipping plot.")
        return

    sizes = sorted({r["size"] for r in records})
    for method in methods:
        subset = sorted((r for r in records if r["method"] == method), key=lambda r: r["size"])
        plt.plot([r["size"] for r in subset], [r["seconds"] for r in subset], marker="o", label=method)

    plt.xscale("log", base=2)
    plt.xticks(sizes, sizes)
    plt.xlabel("Matrix size (N x N)")
    plt.ylabel("Execution time (s)")
    plt.title("Matrix multiplication performance")
    plt.grid(True, which="both", linestyle="--", alpha=0.4)
    plt.legend()
    plt.tight_layout()
    plt.savefig(plot_file, dpi=200)
    print(f"[plot] Saved plot to {plot_file}")



def plot_log(records: List[dict], plot_file: Path) -> None:
    try:
        import matplotlib.pyplot as plt
    except ImportError:  # pragma: no cover - only triggered when matplotlib missing
        print("matplotlib is not installed; skipping plot. Install it with `pip install matplotlib`.", file=sys.stderr)
        return

    plot_file.parent.mkdir(parents=True, exist_ok=True)
    plt.figure(figsize=(10, 6))
    methods = sorted({r["method"] for r in records})
    if not methods:
        print("No timing data found; skipping plot.")
        return

    sizes = sorted({r["size"] for r in records})
    for method in methods:
        subset = sorted((r for r in records if r["method"] == method), key=lambda r: r["size"])
        plt.plot([r["size"] for r in subset], [np.log(r["seconds"]) for r in subset], marker="o", label=method)

    plt.xscale("log", base=2)
    plt.xticks(sizes, sizes)
    plt.xlabel("Matrix size (N x N)")
    plt.ylabel("Execution time (s) (log scale)")
    plt.title("Matrix multiplication performance")
    plt.grid(True, which="both", linestyle="--", alpha=0.4)
    plt.legend()
    plt.tight_layout()
    plt.savefig(plot_file, dpi=200)
    print(f"[plot] Saved plot to {plot_file}")


def main() -> int:
    args = parse_args()
    base_dir = Path(__file__).resolve().parent
    build_dir = base_dir / "build"
    data_dir = base_dir / "benchmark_data"
    output_dir = base_dir / "benchmark_outputs"
    log_file = (base_dir / args.log_file).resolve()
    csv_file = (base_dir / args.csv_file).resolve()
    plot_file = (base_dir / args.plot_file).resolve()
    log_file.parent.mkdir(parents=True, exist_ok=True)

    try:
        sizes = size_list(args)
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 1

    selected_methods = [m for m in METHODS if not args.methods or m["name"] in args.methods]

    build_binaries(base_dir, args.skip_build)

    log_file.unlink(missing_ok=True)
    rng = np.random.default_rng(args.seed)

    for size in sizes:
        a_path, b_path = generate_inputs(size, data_dir, rng, args.regen_inputs)
        for method in selected_methods:
            try:
                run_method(method, size, a_path, b_path, output_dir, build_dir, log_file)
            except subprocess.CalledProcessError as exc:
                print(f"Command failed ({' '.join(map(str, exc.cmd))}): {exc}", file=sys.stderr)
                return 1
            except Exception as exc:  # catch filesystem/binary errors cleanly
                print(f"Failed to run {method['name']} for size {size}: {exc}", file=sys.stderr)
                return 1

    records = parse_log(log_file)
    if not records:
        print("No timing records were collected.", file=sys.stderr)
        return 1

    write_csv(records, csv_file)
    print(f"[csv] Wrote {len(records)} records to {csv_file}")
    plot(records, plot_file)
    plot_log(records, plot_file.with_name(plot_file.stem + "_log" + plot_file.suffix))
    return 0


if __name__ == "__main__":
    sys.exit(main())
