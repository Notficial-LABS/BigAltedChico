#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Watch C++ sources; rebuild incrementally and refresh the native preview window."""
import fcntl
import os
from pathlib import Path
import signal
import subprocess
import sys
import time

PROJECT = Path(__file__).resolve().parent


def fingerprint():
    files = [PROJECT / "CMakeLists.txt", PROJECT / "build.sh"]
    files += [p for p in (PROJECT / "Source").rglob("*") if p.suffix in {".h", ".cpp", ".mm"}]
    return {str(p): (p.stat().st_mtime_ns, p.stat().st_size) for p in files if p.exists()}


def refresh():
    print("[preview] building", flush=True)
    process = subprocess.Popen(["/bin/bash", str(PROJECT / "build.sh"), "preview"], cwd=PROJECT, start_new_session=True)
    try:
        result = process.wait()
    except KeyboardInterrupt:
        os.killpg(process.pid, signal.SIGTERM)
        process.wait()
        raise
    if result == 0:
        print("[preview] ready — guarda un archivo C++ para actualizar la ventana", flush=True)
    else:
        print("[preview] ready — error de compilación; se conserva la ventana anterior", flush=True)


def main():
    (PROJECT / "Working/build").mkdir(parents=True, exist_ok=True)
    with (PROJECT / "Working/build/preview.lock").open("w") as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            print("Ya hay un preview observando este proyecto.")
            return 0
        state = fingerprint()
        refresh()
        while True:
            time.sleep(0.35)
            changed = fingerprint()
            if changed == state:
                continue
            # Wait until saves have settled to avoid building half-written files.
            while True:
                time.sleep(0.65)
                latest = fingerprint()
                if latest == changed:
                    break
                changed = latest
            state = latest
            refresh()


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\nPreview automático detenido; la última ventana sigue disponible.")
