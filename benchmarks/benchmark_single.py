#!/usr/bin/env python3
"""Benchmark single-client SET/GET throughput against MemDB."""
import argparse
import csv
import time
from pathlib import Path

from benchmark_utils import connect, get_benchmarks_dir, send_command


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Benchmark single-client MemDB throughput")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6379)
    parser.add_argument("--set-count", type=int, default=200)
    parser.add_argument("--get-count", type=int, default=200)
    parser.add_argument("--value-size", type=int, default=64)
    parser.add_argument("--timeout", type=float, default=5.0)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    results_path = get_benchmarks_dir() / "results_single.csv"

    with connect(args.host, args.port, timeout=args.timeout) as sock:
        # Warm up the connection with a simple ping.
        send_command(sock, "PING", timeout=args.timeout)

        start = time.perf_counter()
        latencies = []

        for i in range(args.set_count):
            key = f"single_set_{i}"
            value = "x" * args.value_size
            op_start = time.perf_counter()
            send_command(sock, f"SET {key} {value}", timeout=args.timeout)
            latencies.append((time.perf_counter() - op_start) * 1000.0)

        for i in range(args.get_count):
            key = f"single_set_{i % max(1, args.set_count)}"
            op_start = time.perf_counter()
            send_command(sock, f"GET {key}", timeout=args.timeout)
            latencies.append((time.perf_counter() - op_start) * 1000.0)

        total_time = time.perf_counter() - start
        total_requests = args.set_count + args.get_count
        rps = total_requests / total_time if total_time > 0 else 0.0
        avg_latency = sum(latencies) / len(latencies) if latencies else 0.0

    with results_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["set_count", "get_count", "total_requests", "total_time_s", "requests_per_second", "average_latency_ms"])
        writer.writerow([args.set_count, args.get_count, total_requests, round(total_time, 6), round(rps, 3), round(avg_latency, 3)])

    print(f"Saved single-client benchmark to {results_path}")


if __name__ == "__main__":
    main()
