#!/usr/bin/env python3
"""Benchmark multi-client concurrent throughput against MemDB."""
import argparse
import csv
import threading
import time
from pathlib import Path

from benchmark_utils import connect, get_benchmarks_dir, send_command


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Benchmark multi-client MemDB throughput")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6379)
    parser.add_argument("--client-counts", default="1,2,4,8,16")
    parser.add_argument("--requests-per-client", type=int, default=100)
    parser.add_argument("--value-size", type=int, default=64)
    parser.add_argument("--timeout", type=float, default=5.0)
    return parser.parse_args()


def run_client(host: str, port: int, requests_per_client: int, value_size: int, timeout: float, thread_id: int, start_barrier: threading.Barrier, latencies: list[float]) -> None:
    start_barrier.wait()
    with connect(host, port, timeout=timeout) as sock:
        for i in range(requests_per_client):
            key = f"multi_{thread_id}_{i}"
            value = "x" * value_size
            start = time.perf_counter()
            send_command(sock, f"SET {key} {value}", timeout=timeout)
            latencies.append((time.perf_counter() - start) * 1000.0)

            start = time.perf_counter()
            send_command(sock, f"GET {key}", timeout=timeout)
            latencies.append((time.perf_counter() - start) * 1000.0)


def main() -> None:
    args = parse_args()
    client_counts = [int(item.strip()) for item in args.client_counts.split(",") if item.strip()]
    results_path = get_benchmarks_dir() / "results_multi.csv"

    with results_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["clients", "requests_per_client", "total_requests", "total_time_s", "throughput_rps", "average_latency_ms"])

        for client_count in client_counts:
            latencies: list[float] = []
            start_barrier = threading.Barrier(client_count + 1)
            threads = []
            start_time = time.perf_counter()

            for thread_id in range(client_count):
                thread = threading.Thread(
                    target=run_client,
                    args=(args.host, args.port, args.requests_per_client, args.value_size, args.timeout, thread_id, start_barrier, latencies),
                    daemon=True,
                )
                thread.start()
                threads.append(thread)

            start_barrier.wait()
            for thread in threads:
                thread.join()

            total_time = time.perf_counter() - start_time
            total_requests = client_count * args.requests_per_client * 2
            throughput = total_requests / total_time if total_time > 0 else 0.0
            avg_latency = sum(latencies) / len(latencies) if latencies else 0.0
            writer.writerow([client_count, args.requests_per_client, total_requests, round(total_time, 6), round(throughput, 3), round(avg_latency, 3)])
            print(f"Clients={client_count}: throughput={throughput:.3f} rps, avg latency={avg_latency:.3f} ms")

    print(f"Saved multi-client benchmark to {results_path}")


if __name__ == "__main__":
    main()
