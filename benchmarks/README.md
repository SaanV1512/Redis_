# MemDB Benchmark Suite

This directory contains a reproducible benchmarking suite for MemDB.

## Scripts

- benchmark_single.py: single-client SET/GET throughput
- benchmark_multi.py: concurrent multi-client throughput
- benchmark_latency.py: SET/GET/DEL latency statistics
- benchmark_ttl.py: EXPIRE/TTL correctness checks
- plot_results.py: generate plots and a Markdown report

## Usage

Run the benchmarks in order:

```bash
python3 benchmarks/benchmark_single.py --set-count 200 --get-count 200
python3 benchmarks/benchmark_multi.py --client-counts 1,2,4,8,16 --requests-per-client 100
python3 benchmarks/benchmark_latency.py --samples 50
python3 benchmarks/benchmark_ttl.py
python3 benchmarks/plot_results.py
```

The scripts write CSV results into the benchmarks directory and plots into benchmarks/plots.
