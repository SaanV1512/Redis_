#!/usr/bin/env python3
"""Exercise EXPIRE and TTL behavior and verify expiration semantics."""
import argparse
import time
from pathlib import Path

from benchmark_utils import connect, get_benchmarks_dir, send_command


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Benchmark TTL and expiration behavior")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6379)
    parser.add_argument("--ttl-seconds", type=int, default=2)
    parser.add_argument("--sleep-seconds", type=float, default=2.5)
    parser.add_argument("--timeout", type=float, default=5.0)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    results_path = get_benchmarks_dir() / "results_ttl.txt"

    with connect(args.host, args.port, timeout=args.timeout) as sock:
        send_command(sock, "PING", timeout=args.timeout)
        send_command(sock, "FLUSHDB", timeout=args.timeout)
        send_command(sock, f"SET ttl_key value", timeout=args.timeout)
        send_command(sock, f"EXPIRE ttl_key {args.ttl_seconds}", timeout=args.timeout)

        initial_ttl = send_command(sock, "TTL ttl_key", timeout=args.timeout)
        time.sleep(args.sleep_seconds)
        expired_ttl = send_command(sock, "TTL ttl_key", timeout=args.timeout)
        exists_after = send_command(sock, "GET ttl_key", timeout=args.timeout)

    lines = [
        "TTL benchmark",
        f"host={args.host}",
        f"port={args.port}",
        f"ttl_seconds={args.ttl_seconds}",
        f"initial_ttl={initial_ttl}",
        f"expired_ttl={expired_ttl}",
        f"exists_after_expiry={exists_after}",
    ]
    results_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Saved TTL benchmark log to {results_path}")


if __name__ == "__main__":
    main()
