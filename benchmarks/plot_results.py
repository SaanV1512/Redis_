#!/usr/bin/env python3
"""Generate plots and a benchmark report from the generated CSV data."""
import argparse
import csv
import os
import platform
import shutil
import statistics
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

from benchmark_utils import get_benchmarks_dir


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Plot MemDB benchmark results")
    parser.add_argument("--output-dir", default=None)
    return parser.parse_args()


def ensure_plot_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def load_csv(path: Path) -> pd.DataFrame:
    return pd.read_csv(path) if path.exists() else pd.DataFrame()


def plot_throughput_vs_clients(df: pd.DataFrame, output_path: Path) -> None:
    if df.empty or "clients" not in df.columns or "throughput_rps" not in df.columns:
        return
    plt.figure(figsize=(8, 4.5))
    plt.plot(df["clients"], df["throughput_rps"], marker="o", linewidth=2.2, color="#1f77b4")
    plt.xlabel("Concurrent clients")
    plt.ylabel("Throughput (requests/sec)")
    plt.title("Throughput scaling with concurrency")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(output_path, dpi=200)
    plt.close()


def plot_latency_distribution(df: pd.DataFrame, output_path: Path) -> None:
    if df.empty or "latency_ms" not in df.columns:
        return
    plt.figure(figsize=(8, 4.5))
    plt.hist(df["latency_ms"], bins=20, color="#d62728", edgecolor="black", alpha=0.85)
    plt.xlabel("Latency (ms)")
    plt.ylabel("Frequency")
    plt.title("Latency distribution")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(output_path, dpi=200)
    plt.close()


def plot_requests_per_second(df: pd.DataFrame, output_path: Path) -> None:
    if df.empty or "requests_per_second" not in df.columns:
        return
    plt.figure(figsize=(8, 4.5))
    plt.bar(["single"], [df.iloc[0]["requests_per_second"]], color="#2ca02c")
    plt.ylabel("Requests/sec")
    plt.title("Single-client throughput")
    plt.grid(True, alpha=0.3, axis="y")
    plt.tight_layout()
    plt.savefig(output_path, dpi=200)
    plt.close()


def write_report(output_dir: Path, single_df: pd.DataFrame, multi_df: pd.DataFrame, latency_df: pd.DataFrame, ttl_path: Path) -> None:
    timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S UTC")
    report_path = get_benchmarks_dir() / "report.md"
    hardware = []
    hardware.append(f"OS: {platform.system()} {platform.release()}")
    hardware.append(f"Python: {platform.python_version()}")
    try:
        with open("/proc/cpuinfo", "r", encoding="utf-8") as handle:
            cpu = next((line.split(":", 1)[1].strip() for line in handle if line.startswith("model name")), "unknown")
            hardware.append(f"CPU: {cpu}")
    except Exception:
        hardware.append("CPU: unknown")
    try:
        total_ram_kb = int(open("/proc/meminfo", "r", encoding="utf-8").read().splitlines()[0].split()[1])
        hardware.append(f"RAM: {total_ram_kb / (1024 * 1024):.2f} GB")
    except Exception:
        hardware.append("RAM: unknown")

    single_rps = float(single_df.iloc[0]["requests_per_second"]) if not single_df.empty and "requests_per_second" in single_df.columns else 0.0
    peak_throughput = float(multi_df["throughput_rps"].max()) if not multi_df.empty and "throughput_rps" in multi_df.columns else 0.0
    avg_latency = float(latency_df["mean_ms"].mean()) if not latency_df.empty and "mean_ms" in latency_df.columns else 0.0
    concurrency_scaling = "".join([f"{row['clients']} clients -> {row['throughput_rps']} rps; " for _, row in multi_df.iterrows()]) if not multi_df.empty else "N/A"

    report_lines = [
        "# MemDB Benchmark Report",
        "",
        f"- Timestamp: {timestamp}",
        "- Hardware:",
    ]
    report_lines.extend(f"  - {item}" for item in hardware)
    report_lines.extend([
        "- Benchmark configuration:",
        "  - Host: 127.0.0.1",
        "  - Port: 6379",
        "  - Request counts: configurable via CLI",
        "  - Value size: configurable via CLI",
        "  - Client counts: configurable via CLI",
        "- Summary:",
        f"  - Single-client throughput: {single_rps:.3f} requests/sec",
        f"  - Average latency: {avg_latency:.3f} ms",
        f"  - Peak throughput: {peak_throughput:.3f} requests/sec",
        f"  - Concurrency scaling: {concurrency_scaling}",
        f"  - TTL log: {ttl_path.name}",
    ])
    report_path.write_text("\n".join(report_lines) + "\n", encoding="utf-8")


def main() -> None:
    args = parse_args()
    output_dir = Path(args.output_dir) if args.output_dir else get_benchmarks_dir() / "plots"
    ensure_plot_dir(output_dir)

    single_df = load_csv(get_benchmarks_dir() / "results_single.csv")
    multi_df = load_csv(get_benchmarks_dir() / "results_multi.csv")
    latency_df = load_csv(get_benchmarks_dir() / "results_latency.csv")
    ttl_path = get_benchmarks_dir() / "results_ttl.txt"

    plot_throughput_vs_clients(multi_df, output_dir / "throughput_vs_clients.png")
    plot_latency_distribution(load_csv(get_benchmarks_dir() / "results_latency_samples.csv"), output_dir / "latency_distribution.png")
    plot_requests_per_second(single_df, output_dir / "requests_per_second.png")
    write_report(output_dir, single_df, multi_df, latency_df, ttl_path)

    print(f"Saved plots and report to {output_dir}")


if __name__ == "__main__":
    main()
