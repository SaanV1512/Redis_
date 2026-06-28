# MemDB Benchmark Report

- Timestamp: 2026-06-27 19:29:49 UTC
- Hardware:
  - OS: Linux 6.18.33.1-microsoft-standard-WSL2
  - Python: 3.13.13
  - CPU: AMD Ryzen 7 7445HS w/ Radeon 740M Graphics
  - RAM: 7.38 GB
- Benchmark configuration:
  - Host: 127.0.0.1
  - Port: 6379
  - Request counts: configurable via CLI
  - Value size: configurable via CLI
  - Client counts: configurable via CLI
- Summary:
  - Single-client throughput: 5993.949 requests/sec
  - Average latency: 0.165 ms
  - Peak throughput: 8508.578 requests/sec
  - Concurrency scaling: 1.0 clients -> 5204.135 rps; 2.0 clients -> 8508.578 rps; 4.0 clients -> 7412.567 rps; 8.0 clients -> 5158.137 rps; 16.0 clients -> 4961.296 rps; 
  - TTL log: results_ttl.txt
