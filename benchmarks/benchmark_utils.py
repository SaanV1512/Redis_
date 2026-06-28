import socket
from pathlib import Path


def get_benchmarks_dir() -> Path:
    return Path(__file__).resolve().parent


def connect(host: str, port: int, timeout: float = 5.0) -> socket.socket:
    client = socket.create_connection((host, port), timeout=timeout)
    client.settimeout(timeout)
    return client


def send_command(sock: socket.socket, command: str, timeout: float = 5.0) -> str:
    sock.settimeout(timeout)
    sock.sendall((command + "\n").encode("utf-8"))

    chunks = b""
    while b"\n" not in chunks:
        chunk = sock.recv(4096)
        if not chunk:
            break
        chunks += chunk

    return chunks.decode("utf-8").strip()
