#!/usr/bin/env python3
"""Helper: write a Claude status to the status file for ESP32.

Called by Claude Code hooks, e.g.:
    python status_setter.py IDLE
    python status_setter.py THINK
    python status_setter.py CODE
    python status_setter.py RUN
    python status_setter.py ERROR
"""

import sys
import os

STATUS_FILE = os.path.expanduser(r"~\.claude\claude_status")

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <STATE>", file=sys.stderr)
        sys.exit(1)

    state = sys.argv[1].strip().upper()
    valid = {"IDLE", "THINK", "CODE", "RUN", "ERROR"}
    if state not in valid:
        print(f"Invalid state: {state}. Valid: {valid}", file=sys.stderr)
        sys.exit(2)

    os.makedirs(os.path.dirname(STATUS_FILE), exist_ok=True)
    with open(STATUS_FILE, "w") as f:
        f.write(state + "\n")
    print(f"[OK] Status → {state}")

if __name__ == "__main__":
    main()
