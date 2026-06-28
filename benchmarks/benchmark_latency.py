#!/usr/bin/env python3
"""Measure per-operation latency for SET/GET/DEL against MemDB."""
import argparse
import csv
import statistics
import time
from pathlib import Path

from benchmark_utils import connect, get_benchmarks_dir, send_command


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Measure MemDB operation latency")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6379)
    parser.add_argument("--samples", type=int, default=50)
    parser.add_argument("--value-size", type=int, default=64)
    parser.add_argument("--timeout", type=float, default=5.0)
    return parser.parse_args()


def benchmark_one_operation(sock, operation: str, key: str, value: str, timeout: float) -> float:
    start = time.perf_counter()
    if operation == "SET":
        send_command(sock, f"SET {key} {value}", timeout=timeout)
    elif operation == "GET":
        send_command(sock, f"GET {key}", timeout=timeout)
    elif operation == "DEL":
        send_command(sock, f"DEL {key}", timeout=timeout)
    else:
        raise ValueError(f"Unsupported operation: {operation}")
    return (time.perf_counter() - start) * 1000.0


def main() -> None:
    args = parse_args()
    results_path = get_benchmarks_dir() / "results_latency.csv"
    samples_path = get_benchmarks_dir() / "results_latency_samples.csv"

    with connect(args.host, args.port, timeout=args.timeout) as sock:
        send_command(sock, "PING", timeout=args.timeout)
        operations = ["SET", "GET", "DEL"]
        with results_path.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.writer(handle)
            writer.writerow(["operation", "samples", "mean_ms", "median_ms", "min_ms", "max_ms", "stddev_ms"])

            with samples_path.open("w", newline="", encoding="utf-8") as sample_handle:
                sample_writer = csv.writer(sample_handle)
                sample_writer.writerow(["operation", "latency_ms"])

                for operation in operations:
                    latencies = []
                    for index in range(args.samples):
                        key = f"latency_{operation.lower()}_{index}"
                        value = "v" * args.value_size
                        if operation == "GET":
                            send_command(sock, f"SET {key} {value}", timeout=args.timeout)
                        elif operation == "DEL":
                            send_command(sock, f"SET {key} {value}", timeout=args.timeout)
                        latency = benchmark_one_operation(sock, operation, key, value, args.timeout)
                        latencies.append(latency)
                        sample_writer.writerow([operation, round(latency, 6)])

                    writer.writerow([
                        operation,
                        len(latencies),
                        round(statistics.mean(latencies), 6),
                        round(statistics.median(latencies), 6),
                        round(min(latencies), 6),
                        round(max(latencies), 6),
                        round(statistics.pstdev(latencies), 6),
                    ])

    print(f"Saved latency benchmarks to {results_path} and {samples_path}")


if __name__ == "__main__":
    main()
